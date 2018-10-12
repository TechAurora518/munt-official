// Copyright (c) 2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

package com.gulden.unity_wallet

import android.app.Application
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.support.design.widget.BottomNavigationView
import android.support.v4.app.Fragment
import android.support.v4.app.FragmentManager
import android.support.v4.app.FragmentTransaction
import android.support.v7.app.AppCompatActivity
import android.view.View
import com.google.android.gms.common.api.CommonStatusCodes
import com.google.android.gms.vision.barcode.Barcode
import com.gulden.barcodereader.BarcodeCaptureActivity
import com.gulden.jniunifiedbackend.*
import com.gulden.unity_wallet.MainActivityFragments.ReceiveFragment
import com.gulden.unity_wallet.MainActivityFragments.SendFragment
import com.gulden.unity_wallet.MainActivityFragments.SendFragment.OnFragmentInteractionListener
import com.gulden.unity_wallet.MainActivityFragments.SettingsFragment
import com.gulden.unity_wallet.MainActivityFragments.TransactionFragment
import kotlinx.android.synthetic.main.activity_main.*
import java.util.*
import com.gulden.unity_wallet.ui.buy.BuyActivity


inline fun FragmentManager.inTransaction(func: FragmentTransaction.() -> FragmentTransaction) {
    beginTransaction().func().commit()
}

fun AppCompatActivity.addFragment(fragment: Fragment, frameId: Int){
    supportFragmentManager.inTransaction { add(frameId, fragment) }
}

fun AppCompatActivity.replaceFragment(fragment: Any, frameId: Int) {
    supportFragmentManager.inTransaction{replace(frameId, fragment as Fragment)}
}

class MainActivity : AppCompatActivity(), OnFragmentInteractionListener, ReceiveFragment.OnFragmentInteractionListener, TransactionFragment.OnFragmentInteractionListener, SettingsFragment.OnFragmentInteractionListener
{
    fun setSyncProgress(percent: Float)
    {
        syncProgress.progress = (1000000 * (percent/100)).toInt();
    }

    fun setWalletBalance(balance : Long)
    {
        walletBalance.text = (balance.toFloat() / 100000000).toString()
        walletBalanceLogo.visibility = View.VISIBLE;
        walletBalance.visibility = View.VISIBLE;
        walletLogo.visibility = View.GONE;
    }

    fun coreUIInit()
    {
        addFragment(sendFragment, R.id.mainLayout)
    }

    fun createWallet()
    {
        // Show the 'welcome' activity.
        val intent = Intent(this, WelcomeActivity::class.java)
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK)
        startActivity(intent)

        finish()
    }

    override fun onFragmentInteraction(uri: Uri)
    {

    }

    private val sendFragment : SendFragment = SendFragment();
    private val receiveFragment : ReceiveFragment = ReceiveFragment();
    private val transactionFragment : TransactionFragment = TransactionFragment();
    private val settingsFragment : SettingsFragment = SettingsFragment();

    private val mOnNavigationItemSelectedListener = BottomNavigationView.OnNavigationItemSelectedListener { item ->
         when (item.itemId) {
            R.id.navigation_send -> {
                replaceFragment(sendFragment, R.id.mainLayout)
                return@OnNavigationItemSelectedListener true
            }
            R.id.navigation_receive -> {
                replaceFragment(receiveFragment, R.id.mainLayout)
                return@OnNavigationItemSelectedListener true
            }
            R.id.navigation_transactions -> {
                replaceFragment(transactionFragment, R.id.mainLayout)
                return@OnNavigationItemSelectedListener true
            }
            R.id.navigation_settings -> {
                replaceFragment(settingsFragment, R.id.mainLayout)
                return@OnNavigationItemSelectedListener true
            }
        }
        false
    }



    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        navigation.setOnNavigationItemSelectedListener(mOnNavigationItemSelectedListener)

        syncProgress.max = 1000000;
        syncProgress.progress = 0;

        (application as ActivityManager).mainActivity = this;
    }

    fun handleQRScanButtonClick(view : View) {
        val intent = Intent(applicationContext, BarcodeCaptureActivity::class.java)
        startActivityForResult(intent, BARCODE_READER_REQUEST_CODE)
    }

    fun gotoBuyActivity(_view : View)
    {
        gotoBuyActivity()
    }

    fun gotoBuyActivity()
    {
        val intent = Intent(this, BuyActivity::class.java)
        intent.putExtra(BuyActivity.ARG_BUY_ADDRESS, GuldenUnifiedBackend.GetReceiveAddress().toString())
        startActivityForResult(intent, BUY_RETURN_CODE)
    }

    fun Uri.getParameters(): HashMap<String, String> {
        val items : HashMap<String, String> = HashMap<String, String>();
        if (isOpaque())
            return items;

        val query = encodedQuery ?: return items;

        var start = 0
        do {
            val nextAmpersand = query.indexOf('&', start)
            val end = if (nextAmpersand != -1) nextAmpersand else query.length

            var separator = query.indexOf('=', start)
            if (separator > end || separator == -1) {
                separator = end
            }

            if (separator == end) {
                items[Uri.decode(query.substring(start, separator))] = "";
            } else {
                items[Uri.decode(query.substring(start, separator))] = Uri.decode(query.substring(separator + 1, end));
            }

            // Move start to end of name.
            if (nextAmpersand != -1) {
                start = nextAmpersand + 1
            } else {
                break
            }
        } while (true)
        return items
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == BARCODE_READER_REQUEST_CODE) {
            if (resultCode == CommonStatusCodes.SUCCESS)
            {
                if (data != null) {
                    val barcode = data.getParcelableExtra<Barcode>(BarcodeCaptureActivity.BarcodeObject)

                    var barcodeText = barcode.displayValue;
                    var parsedQRCodeURI = Uri.parse(barcodeText);
                    var address : String = "";

                    // Handle all possible scheme variations (foo: foo:// etc.)
                    if ((parsedQRCodeURI?.authority == null) && (parsedQRCodeURI?.path == null))
                    {
                        parsedQRCodeURI = Uri.parse(barcodeText.replaceFirst(":", "://"));
                    }
                    if (parsedQRCodeURI?.authority != null) address += parsedQRCodeURI?.authority;
                    if (parsedQRCodeURI?.path != null) address += parsedQRCodeURI?.path;

                    val parsedQRCodeURIRecord = UriRecord(parsedQRCodeURI.scheme, address , parsedQRCodeURI.getParameters())
                    val recipient = GuldenUnifiedBackend.IsValidRecipient(parsedQRCodeURIRecord);
                    if (recipient.valid) {
                        val intent = Intent(applicationContext, SendCoinsActivity::class.java)
                        intent.putExtra(SendCoinsActivity.EXTRA_RECIPIENT, recipient);
                        startActivityForResult(intent, SEND_COINS_RETURN_CODE)
                    }
                }
            }
        } else {
            super.onActivityResult(requestCode, resultCode, data)
        }
    }

    public fun setFocusOnAddress(view : View)
    {
        receiveFragment?.setFocusOnAddress();
    }

    companion object {
        private val BARCODE_READER_REQUEST_CODE = 1
        public val SEND_COINS_RETURN_CODE = 2
        public val BUY_RETURN_CODE = 3
    }
}
