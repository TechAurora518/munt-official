// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

package com.gulden.unity_wallet.main_activity_fragments

import android.content.Context
import android.net.Uri
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.gulden.jniunifiedbackend.GuldenUnifiedBackend
import com.gulden.unity_wallet.R
import com.gulden.unity_wallet.ui.MutationAdapter
import kotlinx.android.synthetic.main.fragment_mutation.*
import android.content.Intent
import com.gulden.jniunifiedbackend.MutationRecord
import com.gulden.jniunifiedbackend.TransactionRecord
import com.gulden.unity_wallet.TransactionInfoActivity
import com.gulden.unity_wallet.UnityCore
import org.jetbrains.anko.support.v4.runOnUiThread


class MutationFragment : androidx.fragment.app.Fragment(), UnityCore.Observer {

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View?
    {
        return inflater.inflate(R.layout.fragment_mutation, container, false)
    }

    override fun onActivityCreated(savedInstanceState: Bundle?)
    {
        super.onActivityCreated(savedInstanceState)

        mutationList?.emptyView = emptyMutationListView

        val mutations = GuldenUnifiedBackend.getMutationHistory()

        val adapter = MutationAdapter(this.context!!, mutations)
        mutationList.adapter = adapter

        mutationList.setOnItemClickListener { parent, _, position, _ ->
            val mutation = parent.adapter.getItem(position) as MutationRecord
            val intent = Intent(this.context, TransactionInfoActivity::class.java)
            intent.putExtra(TransactionInfoActivity.EXTRA_TRANSACTION, mutation.txHash)
            startActivity(intent)
        }
    }

    override fun onAttach(context: Context)
    {
        UnityCore.instance.addObserver(this)

        super.onAttach(context)
        if (context is OnFragmentInteractionListener)
        {
            listener = context
        }
        else
        {
            throw RuntimeException("$context must implement OnFragmentInteractionListener")
        }
    }

    override fun onDetach() {
        UnityCore.instance.removeObserver(this)

        super.onDetach()
        listener = null
    }

    override fun incomingTransaction(transaction: TransactionRecord): Boolean {
        val mutations = GuldenUnifiedBackend.getMutationHistory()
        runOnUiThread {
            val adapter = mutationList.adapter as MutationAdapter
            adapter.updateDataSource(mutations)
        }
        return true
    }

    private var listener: OnFragmentInteractionListener? = null
    interface OnFragmentInteractionListener
    {
        fun onFragmentInteraction(uri: Uri)
    }
}
