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

data class UnityConfig(val dataDir: String, val testnet: Boolean)

class UnityCore {
    interface Observer {
        fun syncProgressChanged(percent: Float): Boolean { return false }
        fun walletBalanceChanged(balance: Long): Boolean { return false }
        fun onCoreReady(): Boolean { return false }
        fun onCoreShutdown(): Boolean { return false }
        fun createNewWallet(): Boolean { return false }
        fun haveExistingWallet(): Boolean { return false }
        fun onNewMutation(mutation: MutationRecord) {}
        fun updatedTransaction(transaction: TransactionRecord): Boolean { return false }
    }

    companion object {
        val instance: UnityCore = UnityCore()
    }

    @Synchronized
    fun configure(_config: UnityConfig) {
        if (config != null)
            throw RuntimeException("Can only configure once")
        config = _config
    }

    fun addObserver(observer: Observer) {
        observersLock.withLock {
            observers.add(observer)
        }
    }

    fun removeObserver(observer: Observer) {
        observersLock.withLock {
            observers.remove(observer)
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
                GuldenUnifiedBackend.InitUnityLib(cfg.dataDir, cfg.testnet, coreLibrarySignalHandler)
            }

            started = true
        }
    }

    fun isCoreReady(): Boolean {
        return coreReady
    }

    private var config: UnityConfig? = null
    private var started: Boolean = false
    private var coreReady: Boolean = false

    private var observersLock: Lock = ReentrantLock()
    private var observers: MutableSet<Observer> = mutableSetOf()

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
                    it.syncProgressChanged(percent)
                }
            }
        }

        override fun notifyBalanceChange(newBalance: BalanceRecord): Boolean {
            balanceAmount = newBalance.availableIncludingLocked + newBalance.immatureIncludingLocked + newBalance.unconfirmedIncludingLocked
            observersLock.withLock {
                observers.forEach {
                    it.walletBalanceChanged(balanceAmount)
                }
            }
            return true
        }

        override fun notifyNewMutation(mutation: MutationRecord) {
            observersLock.withLock {
                observers.forEach {
                    it.onNewMutation(mutation)
                }
            }
        }

        override fun notifyUpdatedTransaction(transaction: TransactionRecord): Boolean {
            observersLock.withLock {
                observers.forEach {
                    it.updatedTransaction(transaction)
                }
            }
            return true
        }

        override fun notifyShutdown(): Boolean {
            observersLock.withLock {
                observers.forEach {
                    it.onCoreShutdown()
                }
            }
            return true
        }

        override fun notifyCoreReady(): Boolean {
            coreReady = true
            observers.forEach {
                it.onCoreReady()
            }
            return true
        }

        override fun notifyInitWithExistingWallet() {
            observersLock.withLock {
                observers.forEach {
                    it.haveExistingWallet()
                }
            }
        }

        override fun notifyInitWithoutExistingWallet() {
            observersLock.withLock {
                observers.forEach {
                    it.createNewWallet()
                }
            }
        }
    }

}
