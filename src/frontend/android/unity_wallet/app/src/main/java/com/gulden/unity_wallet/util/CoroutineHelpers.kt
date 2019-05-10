package com.gulden.unity_wallet.util

import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

fun <T> CompletableDeferred<T>.invokeNowOrOnSuccesfullCompletion(scope: CoroutineScope, func: () -> Unit) {
    if (isCompleted && !isCancelled)
        func()
    else {
        invokeOnCompletion { handler ->
            if (handler == null) {
                scope.launch(Dispatchers.Main) { func() }
            }
        }
    }
}