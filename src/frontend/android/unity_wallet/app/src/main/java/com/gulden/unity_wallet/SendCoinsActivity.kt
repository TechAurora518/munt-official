// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

package com.gulden.unity_wallet

import android.os.Bundle
import android.support.design.widget.Snackbar
import android.support.v7.app.AppCompatActivity
import android.view.View
import com.gulden.jniunifiedbackend.AddressRecord
import com.gulden.jniunifiedbackend.GuldenUnifiedBackend
import com.gulden.jniunifiedbackend.UriRecipient

import kotlinx.android.synthetic.main.activity_send_coins.*
import android.content.Context
import android.support.v7.app.AlertDialog
import android.widget.EditText
import android.view.ViewGroup
import android.view.LayoutInflater
import com.gulden.unity_wallet.R.layout.text_input_address_label
import kotlinx.android.synthetic.main.text_input_address_label.view.*


class SendCoinsActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_send_coins)
        setSupportActionBar(toolbar)

        var recipient : UriRecipient = intent.getParcelableExtra(EXTRA_RECIPIENT)
        send_coins_amount.setText(recipient.amount)
        send_coins_receiving_static_address.text = recipient.address

        setAddressLabel(recipient.label)

        fab.setOnClickListener {
            view -> run {
                if (send_coins_amount.text.length > 0) {
                    var paymentRequest : UriRecipient = UriRecipient(true, recipient.address, recipient.label, send_coins_amount.text.toString())
                    if (GuldenUnifiedBackend.performPaymentToRecipient(paymentRequest)) {
                        finish()
                    }
                    else {
                        Snackbar.make(view, "Payment failed", Snackbar.LENGTH_LONG).setAction("Action", null).show()
                    }
                }
                else {
                    Snackbar.make(view, "Enter an amount to pay", Snackbar.LENGTH_LONG).setAction("Action", null).show()
                }
            }
        }
    }

    fun setAddressLabel(label : String)
    {
        send_coins_receiving_static_label.text = label
        setAddressHasLabel(label.isNotEmpty())
    }

    fun setAddressHasLabel(hasLabel : Boolean)
    {
        if (hasLabel)
        {
            send_coins_receiving_static_label.visibility = View.VISIBLE
            labelRemoveFromAddressBook.visibility = View.VISIBLE
            labelAddToAddressBook.visibility = View.GONE
        }
        else
        {
            send_coins_receiving_static_label.visibility = View.GONE
            labelRemoveFromAddressBook.visibility = View.GONE
            labelAddToAddressBook.visibility = View.VISIBLE
        }
    }

    fun appendNumberToAmount(number : String)
    {
        if (send_coins_amount.text.toString() == "0")
            send_coins_amount.setText(number)
        else
            send_coins_amount.setText(send_coins_amount.text.toString() + number)
    }

    fun handleKeypadButtonClick(view : View)
    {
        when (view.id)
        {
            R.id.button_1 -> appendNumberToAmount("1")
            R.id.button_2 -> appendNumberToAmount("2")
            R.id.button_3 -> appendNumberToAmount("3")
            R.id.button_4 -> appendNumberToAmount("4")
            R.id.button_5 -> appendNumberToAmount("5")
            R.id.button_6 -> appendNumberToAmount("6")
            R.id.button_7 -> appendNumberToAmount("7")
            R.id.button_8 -> appendNumberToAmount("8")
            R.id.button_9 -> appendNumberToAmount("9")
            R.id.button_0 -> {
                if (send_coins_amount.text.isEmpty())
                    send_coins_amount.setText(send_coins_amount.text.toString() + "0.")
                else if (send_coins_amount.text.toString() != "0")
                    send_coins_amount.setText(send_coins_amount.text.toString() + "0")
            }
            R.id.button_backspace -> {
                if (send_coins_amount.text.toString() == "0.")
                    send_coins_amount.setText("")
                else
                    send_coins_amount.setText(send_coins_amount.text.dropLast(1))
            }
            R.id.button_decimal -> {
                if (!send_coins_amount.text.contains("."))
                {
                    if (send_coins_amount.text.isEmpty())
                        send_coins_amount.setText("0.")
                    else
                        send_coins_amount.setText(send_coins_amount.text.toString() + ".")
                }
            }
        }
    }

    fun handleAddToAddressBookClick(view : View)
    {
        val builder = AlertDialog.Builder(this)
        builder.setTitle("Add address")
        val layoutInflater : LayoutInflater = getSystemService(Context.LAYOUT_INFLATER_SERVICE) as LayoutInflater
        val viewInflated : View = layoutInflater.inflate(text_input_address_label, view.rootView as ViewGroup, false)
        viewInflated.labelAddAddressAddress.text = send_coins_receiving_static_address.text
        val input = viewInflated.findViewById(R.id.input) as EditText
        builder.setView(viewInflated)
        builder.setPositiveButton(android.R.string.ok) { dialog, _ ->
            dialog.dismiss()
            val label = input.text.toString()
            val record = AddressRecord(send_coins_receiving_static_address.text.toString(), "Send", label)
            GuldenUnifiedBackend.addAddressBookRecord(record)
            setAddressLabel(label)
        }
        builder.setNegativeButton(android.R.string.cancel) { dialog, _ -> dialog.cancel() }
        builder.show()
    }

    fun handleRemoveFromAddressBookClick(view : View)
    {
        val record = AddressRecord(send_coins_receiving_static_address.text.toString(), "Send", send_coins_receiving_static_label.text.toString())
        GuldenUnifiedBackend.deleteAddressBookRecord(record)
        setAddressLabel("")
    }

    companion object
    {
        val EXTRA_RECIPIENT = "recipient"
    }
}
