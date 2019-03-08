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
import android.widget.AdapterView
import com.gulden.unity_wallet.R
import com.gulden.unity_wallet.Currencies
import com.gulden.unity_wallet.ui.LocalCurrenciesAdapter
import kotlinx.android.synthetic.main.fragment_local_currencies.*


class LocalCurrencyFragment : androidx.fragment.app.Fragment() {

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View?
    {
        return inflater.inflate(R.layout.fragment_local_currencies, container, false)
    }

    override fun onActivityCreated(savedInstanceState: Bundle?)
    {
        super.onActivityCreated(savedInstanceState)

        val adapter = LocalCurrenciesAdapter(this.context!!, Currencies.knownCurrencies)
        currenciesList.adapter = adapter
        currenciesList.setOnItemClickListener { parent, _, position, _ ->
            adapter.setSelectedPosition(position)
            adapter.notifyDataSetChanged()
        }
    }

}
