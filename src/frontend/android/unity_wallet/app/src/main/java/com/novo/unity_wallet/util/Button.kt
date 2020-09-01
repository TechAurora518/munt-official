// Copyright (c) 2019 The Novo developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the Novo software license, see the accompanying
// file COPYING

package com.novo.unity_wallet.util

import android.widget.Button
import com.novo.unity_wallet.R

// Helper function to visually toggle a button as enabled/disabled while still keeping it clickable
fun setFauxButtonEnabledState(button : Button, enabled : Boolean)
{
    // Toggle button visual disabled/enabled indicator while still keeping it clickable
    var buttonBackground = R.drawable.shape_square_button_disabled
    if (enabled)
        buttonBackground = R.drawable.shape_square_button_enabled
    button.setBackgroundResource(buttonBackground)
}