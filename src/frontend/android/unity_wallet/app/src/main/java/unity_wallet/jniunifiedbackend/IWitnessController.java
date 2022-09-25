// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.atomic.AtomicBoolean;

/** C++ interface to control witness accounts */
public abstract class IWitnessController {
    /** Get information on min/max witness periods, weights etc. */
    public static HashMap<String, String> getNetworkLimits()
    {
        return CppProxy.getNetworkLimits();
    }

    /** Get an estimate of weights/parts that a witness account will be funded with */
    public static WitnessEstimateInfoRecord getEstimatedWeight(long amountToLock, long lockPeriodInBlocks)
    {
        return CppProxy.getEstimatedWeight(amountToLock,
                                           lockPeriodInBlocks);
    }

    /** Fund a witness account */
    public static WitnessFundingResultRecord fundWitnessAccount(String fundingAccountUUID, String witnessAccountUUID, long fundingAmount, long requestedLockPeriodInBlocks)
    {
        return CppProxy.fundWitnessAccount(fundingAccountUUID,
                                           witnessAccountUUID,
                                           fundingAmount,
                                           requestedLockPeriodInBlocks);
    }

    /** Renew a witness account */
    public static WitnessFundingResultRecord renewWitnessAccount(String fundingAccountUUID, String witnessAccountUUID)
    {
        return CppProxy.renewWitnessAccount(fundingAccountUUID,
                                            witnessAccountUUID);
    }

    /** Get information on account weight and other witness statistics for account */
    public static WitnessAccountStatisticsRecord getAccountWitnessStatistics(String witnessAccountUUID)
    {
        return CppProxy.getAccountWitnessStatistics(witnessAccountUUID);
    }

    /** Turn compounding on/off */
    public static void setAccountCompounding(String witnessAccountUUID, int percentToCompount)
    {
        CppProxy.setAccountCompounding(witnessAccountUUID,
                                       percentToCompount);
    }

    /** Check state of compounding; returns a percentage between 1 and 100, or 0 if not compounding */
    public static int isAccountCompounding(String witnessAccountUUID)
    {
        return CppProxy.isAccountCompounding(witnessAccountUUID);
    }

    /** Get the witness address of the account */
    public static String getWitnessAddress(String witnessAccountUUID)
    {
        return CppProxy.getWitnessAddress(witnessAccountUUID);
    }

    /** Get the optimal distribution amounts for the account; totalNetworkWeight should be the value of "total_weight_eligible_raw" */
    public static ArrayList<Long> getOptimalWitnessDistribution(long amount, long durationInBlocks, long totalNetworkWeight)
    {
        return CppProxy.getOptimalWitnessDistribution(amount,
                                                      durationInBlocks,
                                                      totalNetworkWeight);
    }

    /** Same as the above but calculates all the paramaters from the account UUID; its more efficient to use the other call if you already have these values */
    public static ArrayList<Long> getOptimalWitnessDistributionForAccount(String witnessAccountUUID)
    {
        return CppProxy.getOptimalWitnessDistributionForAccount(witnessAccountUUID);
    }

    /** Redistribute a witness account to its optimal distribution, call 'getOptimalWitnessDistribution' first to calculate this */
    public static ResultRecord optimiseWitnessAccount(String witnessAccountUUID, String fundingAccountUUID, ArrayList<Long> optimalDistribution)
    {
        return CppProxy.optimiseWitnessAccount(witnessAccountUUID,
                                               fundingAccountUUID,
                                               optimalDistribution);
    }

    private static final class CppProxy extends IWitnessController
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

        public static native HashMap<String, String> getNetworkLimits();

        public static native WitnessEstimateInfoRecord getEstimatedWeight(long amountToLock, long lockPeriodInBlocks);

        public static native WitnessFundingResultRecord fundWitnessAccount(String fundingAccountUUID, String witnessAccountUUID, long fundingAmount, long requestedLockPeriodInBlocks);

        public static native WitnessFundingResultRecord renewWitnessAccount(String fundingAccountUUID, String witnessAccountUUID);

        public static native WitnessAccountStatisticsRecord getAccountWitnessStatistics(String witnessAccountUUID);

        public static native void setAccountCompounding(String witnessAccountUUID, int percentToCompount);

        public static native int isAccountCompounding(String witnessAccountUUID);

        public static native String getWitnessAddress(String witnessAccountUUID);

        public static native ArrayList<Long> getOptimalWitnessDistribution(long amount, long durationInBlocks, long totalNetworkWeight);

        public static native ArrayList<Long> getOptimalWitnessDistributionForAccount(String witnessAccountUUID);

        public static native ResultRecord optimiseWitnessAccount(String witnessAccountUUID, String fundingAccountUUID, ArrayList<Long> optimalDistribution);
    }
}
