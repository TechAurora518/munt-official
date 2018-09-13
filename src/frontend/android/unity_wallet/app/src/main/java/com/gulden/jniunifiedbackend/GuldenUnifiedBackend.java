// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

import java.util.concurrent.atomic.AtomicBoolean;

/** This interface will be implemented in C++ and can be called from any language. */
public abstract class GuldenUnifiedBackend {
    /** Interface constants */
    public static final int VERSION = 1;

    /**Methods */
    public static int InitUnityLib(String dataDir, GuldenUnifiedFrontend signals)
    {
        return CppProxy.InitUnityLib(dataDir,
                                     signals);
    }

    public static QrcodeRecord QRImageFromString(String qrString, int widthHint)
    {
        return CppProxy.QRImageFromString(qrString,
                                          widthHint);
    }

    private static final class CppProxy extends GuldenUnifiedBackend
    {
        private final long nativeRef;
        private final AtomicBoolean destroyed = new AtomicBoolean(false);

        private CppProxy(long nativeRef)
        {
            if (nativeRef == 0) throw new RuntimeException("nativeRef is zero");
            this.nativeRef = nativeRef;
        }

        private native void nativeDestroy(long nativeRef);
        public void _djinni_private_destroy()
        {
            boolean destroyed = this.destroyed.getAndSet(true);
            if (!destroyed) nativeDestroy(this.nativeRef);
        }
        protected void finalize() throws java.lang.Throwable
        {
            _djinni_private_destroy();
            super.finalize();
        }

        public static native int InitUnityLib(String dataDir, GuldenUnifiedFrontend signals);

        public static native QrcodeRecord QRImageFromString(String qrString, int widthHint);
    }
}
