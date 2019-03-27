// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

package com.gulden.unity_wallet.main_activity_fragments

import android.content.ClipboardManager
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.text.Editable
import android.text.Html
import android.text.SpannableString
import android.text.TextWatcher
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.InputMethodManager
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.android.gms.common.api.CommonStatusCodes
import com.google.android.gms.vision.barcode.Barcode
import com.gulden.barcodereader.BarcodeCaptureActivity
import com.gulden.jniunifiedbackend.AddressRecord
import com.gulden.jniunifiedbackend.GuldenUnifiedBackend
import com.gulden.jniunifiedbackend.UriRecipient
import com.gulden.jniunifiedbackend.UriRecord
import com.gulden.unity_wallet.*
import com.gulden.unity_wallet.ui.AddressBookAdapter
import kotlinx.android.synthetic.main.add_address_entry.view.*
import kotlinx.android.synthetic.main.fragment_send.*
import kotlinx.android.synthetic.main.iban_name_entry.view.*
import org.apache.commons.validator.routines.IBANValidator
import org.jetbrains.anko.alert
import org.jetbrains.anko.appcompat.v7.Appcompat
import org.jetbrains.anko.support.v4.runOnUiThread


class SendFragment : Fragment(), UnityCore.Observer
{

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View?
    {
        return inflater.inflate(R.layout.fragment_send, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        clipboardButton.setOnClickListener {
            val text = clipboardText()
            val recipient = when {
                IBANValidator.getInstance().isValid(text) ->
                    UriRecipient(false, text, "", "")
                GuldenUnifiedBackend.IsValidRecipient(UriRecord("gulden", text, HashMap<String,String>())).valid ->
                    GuldenUnifiedBackend.IsValidRecipient(UriRecord("gulden", text, HashMap<String,String>()))
                uriRecipient(text).valid ->
                    uriRecipient(text)
                else ->
                    null
            }
            if (recipient != null) {
                SendCoinsFragment.newInstance(recipient).show(activity!!.supportFragmentManager, SendCoinsFragment::class.java.simpleName)
            }
            else {
                context?.run {
                    alert(Appcompat, getString(R.string.clipboard_no_valid_address)) {
                        positiveButton(getString(android.R.string.ok)) {}
                    }.show()
                }
            }
        }

        imageViewAddToAddressBook.setOnClickListener {
            handleAddToAddressBookButtonClick(view)
        }

        qrButton.setOnClickListener {
            val intent = Intent(context, BarcodeCaptureActivity::class.java)
            intent.putExtra(BarcodeCaptureActivity.AutoFocus, true)
            startActivityForResult(intent, BARCODE_READER_REQUEST_CODE)
        }

        ClipboardManager.OnPrimaryClipChangedListener { checkClipboardEnable() }
    }

    private fun handleAddToAddressBookButtonClick(view : View)
    {
        val builder = AlertDialog.Builder(this.context!!)
        builder.setTitle(getString(R.string.dialog_title_add_address))
        val layoutInflater : LayoutInflater = this.context!!.getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        val viewInflated : View = layoutInflater.inflate(R.layout.add_address_entry, view.rootView as ViewGroup, false)
        val inputAddress = viewInflated.findViewById(R.id.address) as EditText
        val inputLabel = viewInflated.findViewById(R.id.addressName) as EditText
        builder.setView(viewInflated)
        builder.setPositiveButton(android.R.string.ok) { dialog, _ ->
            val address = inputAddress.text.toString()
            val label = inputLabel.text.toString()
            val record = AddressRecord(address, "Send", label)
            UnityCore.instance.addAddressBookRecord(record)
            dialog.dismiss()
        }

        builder.setNegativeButton(android.R.string.cancel) { dialog, _ -> dialog.cancel() }
        val dialog = builder.create()
        dialog.setOnShowListener {
            viewInflated.address.requestFocus()
            viewInflated.address.post {
                val imm = this.context!!.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
                imm.showSoftInput(viewInflated.address, InputMethodManager.SHOW_IMPLICIT)
            }
        }
        dialog.show()
    }

    fun handleURI(payToURI : Uri, forActivity : WalletActivity)
    {
        //TODO: Improve this, and consider moving more of the work into unity core
        //TODO: Handle amounts passed as paramaters etc.
        val address = if (payToURI.host!=null) payToURI.host else ""
        val amount = if (payToURI.queryParameterNames.contains("amount")) payToURI.getQueryParameter("amount") else "0"
        val label = if (payToURI.queryParameterNames.contains("label")) payToURI.getQueryParameter("label") else ""
        var recipient : UriRecipient? = null
        if (IBANValidator.getInstance().isValid(address))
        {
            recipient = UriRecipient(false, address, label, amount)
        }
        else if (GuldenUnifiedBackend.IsValidRecipient(UriRecord("gulden", address, HashMap<String,String>())).valid)
        {
            recipient = GuldenUnifiedBackend.IsValidRecipient(UriRecord("gulden", address, HashMap<String, String>()))
            recipient = UriRecipient(recipient.valid, recipient.address, if (recipient.label!="") recipient.label else label, amount)
        }
        else if (uriRecipient(address).valid)
        {
            recipient = UriRecipient(true, address, label, amount)
        }

        if (recipient != null)
        {
            SendCoinsFragment.newInstance(recipient).show(forActivity.supportFragmentManager, SendCoinsFragment::class.java.simpleName)
        }
    }

    override fun onResume() {
        super.onResume()
        checkClipboardEnable()
    }

    override fun onActivityCreated(savedInstanceState: Bundle?)
    {
        super.onActivityCreated(savedInstanceState)

        addressBookList?.emptyView = emptyAddressBookView

        addressBookList.setOnItemClickListener { parent, _, position, _ ->
            val address = parent.adapter.getItem(position) as AddressRecord
            val recipient = UriRecipient(true, address.address, address.name, "0")
            SendCoinsFragment.newInstance(recipient).show(activity!!.supportFragmentManager, SendCoinsFragment::class.java.simpleName)
        }

        // TODO: Only update if there has been a change, not always.
        val addresses = GuldenUnifiedBackend.getAddressBookRecords()
        val adapter = AddressBookAdapter(this.context!!, addresses)
        addressBookList.adapter = adapter
    }

    override fun onAttach(context: Context)
    {
        UnityCore.instance.addObserver(this)

        super.onAttach(context)
    }

    override fun onDetach() {
        UnityCore.instance.removeObserver(this)

        super.onDetach()
    }

    private fun clipboardText(): String
    {
        val clipboard = ContextCompat.getSystemService(context!!, ClipboardManager::class.java)
        return (clipboard?.primaryClip?.getItemAt(0)?.coerceToText(context)).toString()
    }

    private fun setClipButtonText(text : String)
    {
        val styledText = SpannableString(Html.fromHtml(getString(R.string.send_fragment_clipboard_label) + "<br/>" + "<small> <font color='"+resources.getColor(R.color.lightText)+"'>" + ellipsizeString(text, 18) + "</font> </small>"))
        clipboardButton.text = styledText
    }

    private fun checkClipboardEnable()
    {
        // Enable clipboard button if it contains a valid IBAN, Gulden address or Uri
        val text = clipboardText()
        when
        {
            IBANValidator.getInstance().isValid(text) || GuldenUnifiedBackend.IsValidRecipient(UriRecord("gulden", text, HashMap<String,String>())).valid ->
            {
                setClipButtonText(text)
            }
            uriRecipient(text).valid ->
            {
                setClipButtonText(uriRecipient(text).address)
            }
            else ->
            {
                clipboardButton.text = getString(R.string.send_fragment_clipboard_label)
            }
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == BARCODE_READER_REQUEST_CODE) {
            if (resultCode == CommonStatusCodes.SUCCESS)
            {
                if (data != null) {
                    val barcode = data.getParcelableExtra<Barcode>(BarcodeCaptureActivity.BarcodeObject)
                    val recipient = uriRecipient(barcode.displayValue)
                    if (recipient.valid) {
                        SendCoinsFragment.newInstance(recipient).show(activity!!.supportFragmentManager, SendCoinsFragment::class.java.simpleName)
                    }
                }
            }
        } else {
            super.onActivityResult(requestCode, resultCode, data)
        }
    }

    override fun onAddressBookChanged() {
        val addresses = GuldenUnifiedBackend.getAddressBookRecords()
        runOnUiThread {
            val adapter = addressBookList.adapter as AddressBookAdapter
            adapter.updateDataSource(addresses)
        }
    }

    companion object {
        private const val BARCODE_READER_REQUEST_CODE = 1
    }
}
