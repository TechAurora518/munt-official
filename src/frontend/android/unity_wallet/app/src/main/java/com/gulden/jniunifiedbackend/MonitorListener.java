// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

/** Monitoring events */
public abstract class MonitorListener {
    public abstract void onPartialChain(int height, int probableHeight, int offset);

    public abstract void onPruned(int height);

    public abstract void onProcessedSPVBlocks(int height);
}
