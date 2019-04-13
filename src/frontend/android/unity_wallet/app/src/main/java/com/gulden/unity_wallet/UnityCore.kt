// Copyright (c) 2018 The Gulden developers
// Authored by: Willem de Jonge (willem@isnapp.nl)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

package com.gulden.unity_wallet

import com.gulden.jniunifiedbackend.*
import java.lang.RuntimeException
import java.util.concurrent.locks.Lock
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread
import kotlin.concurrent.withLock

data class UnityConfig(val dataDir: String, val apkPath: String, val staticFilterOffset: Long, val staticFilterLength: Long, val testnet: Boolean)

class UnityCore {
    interface Observer {
        fun syncProgressChanged(percent: Float): Boolean { return false }
        fun walletBalanceChanged(balance: Long): Boolean { return false }
        fun onCoreReady(): Boolean { return false }
        fun onCoreShutdown(): Boolean { return false }
        fun createNewWallet(): Boolean { return false }
        fun haveExistingWallet(): Boolean { return false }
        fun onNewMutation(mutation: MutationRecord, selfCommitted: Boolean) {}
        fun updatedTransaction(transaction: TransactionRecord): Boolean { return false }
        fun onAddressBookChanged() {}
    }

    companion object {
        val instance: UnityCore = UnityCore()
        // Have we previously received an "existing wallet" event at any point
        // Some parts of the codebase need to check this in case it happened before they registered as a listener
        var receivedExistingWalletEvent = false
        // Have we previously received a "create new wallet" event at any point
        // Some parts of the codebase need to check this in case it happened before they registered as a listener
        var receivedCreateNewWalletEvent = false
        // Have we previously been started
        var started = false
    }

    @Synchronized
    fun configure(_config: UnityConfig) {
        if (config != null)
            throw RuntimeException("Can only configure once")
        config = _config
    }

    fun addObserver(observer: Observer, wrapper: (() -> Unit)-> Unit = fun (body: () -> Unit) { body() }) {
        observersLock.withLock {
            observers.add(ObserverEntry(observer, wrapper))
        }
    }

    fun removeObserver(observer: Observer) {
        observersLock.withLock {
            observers.removeAll(
                    observers.filter { it.observer == observer }
            )
        }
    }

    @Synchronized
    fun startCore() {
        if (!started) {
            if (config == null)
                throw RuntimeException("Configure before starting Unity core")

            val cfg: UnityConfig = config as UnityConfig

            thread(true)
            {
                System.loadLibrary("gulden_unity_jni")
                GuldenUnifiedBackend.InitUnityLib(cfg.dataDir, cfg.apkPath, cfg.staticFilterOffset, cfg.staticFilterLength, cfg.testnet, coreLibrarySignalHandler, "")
            }

            started = true
        }
    }

    fun isCoreReady(): Boolean {
        return coreReady
    }

    private var config: UnityConfig? = null
    private var coreReady: Boolean = false

    class ObserverEntry(val observer: Observer, val wrapper: (() ->Unit) -> Unit)
    private var observersLock: Lock = ReentrantLock()
    private var observers: MutableSet<ObserverEntry> = mutableSetOf()

    private var stateTrackLock:  Lock = ReentrantLock()

    var progressPercent: Float = 0F
        set(value) {
            stateTrackLock.withLock {
                field = value
            }
        }
        get() {
            stateTrackLock.withLock {
                return field
            }
        }

    var balanceAmount: Long = 0
        set(value) {
            stateTrackLock.withLock {
                field = value
            }
        }
        get() {
            stateTrackLock.withLock {
                return field
            }
        }

    // TODO wrappers here could be moved into core later
    fun addAddressBookRecord(record: AddressRecord) {
        GuldenUnifiedBackend.addAddressBookRecord(record)
        coreLibrarySignalHandler.notifyAddressBookChanged()
    }

    fun deleteAddressBookRecord(record: AddressRecord) {
        GuldenUnifiedBackend.deleteAddressBookRecord(record)
        coreLibrarySignalHandler.notifyAddressBookChanged()
    }

    // Handle signals from core library, convert and broadcast to all registered observers
    private val coreLibrarySignalHandler = object : GuldenUnifiedFrontend() {
        override fun logPrint(str: String?) {
            // logging already done at C++ level
        }

        override fun notifyUnifiedProgress(progress: Float) {
            val percent: Float = 100 * progress
            progressPercent = percent

            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.syncProgressChanged(percent) }
                }
            }
        }

        override fun notifyBalanceChange(newBalance: BalanceRecord): Boolean {
            balanceAmount = newBalance.availableIncludingLocked + newBalance.immatureIncludingLocked + newBalance.unconfirmedIncludingLocked
            observersLock.withLock {
                observers.forEach {
                    it.wrapper {
                        it.observer.walletBalanceChanged(balanceAmount)
                    }
                }
            }
            return true
        }

        override fun notifyNewMutation(mutation: MutationRecord, selfCommitted: Boolean) {
            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.onNewMutation(mutation, selfCommitted) }
                }
            }
        }

        override fun notifyUpdatedTransaction(transaction: TransactionRecord): Boolean {
            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.updatedTransaction(transaction) }
                }
            }
            return true
        }

        override fun notifyShutdown(): Boolean {
            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.onCoreShutdown() }
                }
            }
            return true
        }

        override fun notifyCoreReady(): Boolean {
            coreReady = true
            observers.forEach {
                it.wrapper { it.observer.onCoreReady() }
            }
            return true
        }

        override fun notifyInitWithExistingWallet() {
            receivedExistingWalletEvent = true
            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.haveExistingWallet() }
                }
            }
        }

        override fun notifyInitWithoutExistingWallet() {
            receivedCreateNewWalletEvent = true
            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.createNewWallet() }
                }
            }
        }

        /*override*/ fun notifyAddressBookChanged() {
            observersLock.withLock {
                observers.forEach {
                    it.wrapper { it.observer.onAddressBookChanged() }
                }
            }
        }
    }

}
