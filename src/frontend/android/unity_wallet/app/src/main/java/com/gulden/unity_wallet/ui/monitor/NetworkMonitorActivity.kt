// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

package com.gulden.unity_wallet.ui.monitor

import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.view.View
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.gulden.unity_wallet.*
import com.gulden.unity_wallet.util.AppBaseActivity
import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.coroutines.Job

class NetworkMonitorActivity : UnityCore.Observer, AppBaseActivity()
{
    override fun onCreate(savedInstanceState: Bundle?)
    {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_network_monitor)

        navigation.setOnNavigationItemSelectedListener(mOnNavigationItemSelectedListener)

        syncProgress.max = 1000000

        if (peersFragment == null)
            peersFragment = PeerListFragment()
        addFragment(peersFragment!!, R.id.networkMonitorMainLayout)
    }

    override fun onDestroy() {
        super.onDestroy()

        coroutineContext[Job]!!.cancel()
    }

    override fun onStart() {
        super.onStart()

        setSyncProgress(UnityCore.instance.progressPercent)
        UnityCore.instance.addObserver(this, fun (callback:() -> Unit) { runOnUiThread { callback() }})
    }

    override fun onStop() {
        super.onStop()
        UnityCore.instance.removeObserver(this)
    }

    fun onBackButtonPushed(view : View) {
        finish()
    }

    private fun gotoPeersPage()
    {
        if (peersFragment == null)
            peersFragment = PeerListFragment()
        replaceFragment(peersFragment!!, R.id.networkMonitorMainLayout)
    }

    private fun gotoBlocksPage()
    {
        if (blocksFragment == null)
            blocksFragment = BlockListFragment()
        replaceFragment(blocksFragment!!, R.id.networkMonitorMainLayout)
    }

    private fun gotoProcessingPage()
    {
        if (processingFragment == null)
            processingFragment = ProcessingFragment()
        replaceFragment(processingFragment!!, R.id.networkMonitorMainLayout)
    }

    private var blocksFragment : BlockListFragment ?= null
    private var peersFragment : PeerListFragment?= null
    private var processingFragment : ProcessingFragment?= null


    private val mOnNavigationItemSelectedListener = BottomNavigationView.OnNavigationItemSelectedListener { item ->
        when (item.itemId) {
            R.id.navigation_peers -> { gotoPeersPage(); return@OnNavigationItemSelectedListener true }
            R.id.navigation_blocks -> { gotoBlocksPage(); return@OnNavigationItemSelectedListener true }
            R.id.navigation_processing -> { gotoProcessingPage(); return@OnNavigationItemSelectedListener true }
        }
        false
    }


    override fun syncProgressChanged(percent: Float) {
        setSyncProgress(percent)
    }

    private fun setSyncProgress(percent: Float)
    {
        syncProgress.progress = (syncProgress.max * (percent/100)).toInt()
        if (percent < 100.0)
            syncProgress.visibility = View.VISIBLE
        else
            syncProgress.visibility = View.INVISIBLE
    }
}
