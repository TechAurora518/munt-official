// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

/** Interface to receive updates about network status */
public abstract class IP2pNetworkListener {
    /** Notify that p2p networking has been enabled */
    public abstract void onNetworkEnabled();

    /** Notify that p2p networking has been disabled */
    public abstract void onNetworkDisabled();

    /** Notify that number of peers has changed */
    public abstract void onConnectionCountChanged(int numConnections);

    /** Notify that amount of data sent/received has changed */
    public abstract void onBytesChanged(int totalRecv, int totalSent);
}
