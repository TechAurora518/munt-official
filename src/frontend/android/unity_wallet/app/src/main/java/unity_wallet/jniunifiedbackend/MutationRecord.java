// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package unity_wallet.jniunifiedbackend;

public final class MutationRecord {


    /*package*/ final long mChange;

    /*package*/ final long mTimestamp;

    /*package*/ final String mTxHash;

    /*package*/ final String mRecipientAddresses;

    /*package*/ final TransactionStatus mStatus;

    /*package*/ final int mDepth;

    public MutationRecord(
            long change,
            long timestamp,
            String txHash,
            String recipientAddresses,
            TransactionStatus status,
            int depth) {
        this.mChange = change;
        this.mTimestamp = timestamp;
        this.mTxHash = txHash;
        this.mRecipientAddresses = recipientAddresses;
        this.mStatus = status;
        this.mDepth = depth;
    }

    public long getChange() {
        return mChange;
    }

    public long getTimestamp() {
        return mTimestamp;
    }

    public String getTxHash() {
        return mTxHash;
    }

    /** Address(es) of transaction recipient(s) (if transaction is sent by us then this excludes e.g. change and only has other wallets addresses) */
    public String getRecipientAddresses() {
        return mRecipientAddresses;
    }

    public TransactionStatus getStatus() {
        return mStatus;
    }

    public int getDepth() {
        return mDepth;
    }

    @Override
    public String toString() {
        return "MutationRecord{" +
                "mChange=" + mChange +
                "," + "mTimestamp=" + mTimestamp +
                "," + "mTxHash=" + mTxHash +
                "," + "mRecipientAddresses=" + mRecipientAddresses +
                "," + "mStatus=" + mStatus +
                "," + "mDepth=" + mDepth +
        "}";
    }

}
