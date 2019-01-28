// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

package com.gulden.jniunifiedbackend;

import java.util.ArrayList;

public final class TransactionRecord implements android.os.Parcelable {


    /*package*/ final String mTxHash;

    /*package*/ final long mTimestamp;

    /*package*/ final long mAmount;

    /*package*/ final long mFee;

    /*package*/ final TransactionStatus mStatus;

    /*package*/ final int mHeight;

    /*package*/ final int mDepth;

    /*package*/ final ArrayList<OutputRecord> mReceivedOutputs;

    /*package*/ final ArrayList<OutputRecord> mSentOutputs;

    public TransactionRecord(
            String txHash,
            long timestamp,
            long amount,
            long fee,
            TransactionStatus status,
            int height,
            int depth,
            ArrayList<OutputRecord> receivedOutputs,
            ArrayList<OutputRecord> sentOutputs) {
        this.mTxHash = txHash;
        this.mTimestamp = timestamp;
        this.mAmount = amount;
        this.mFee = fee;
        this.mStatus = status;
        this.mHeight = height;
        this.mDepth = depth;
        this.mReceivedOutputs = receivedOutputs;
        this.mSentOutputs = sentOutputs;
    }

    public String getTxHash() {
        return mTxHash;
    }

    public long getTimestamp() {
        return mTimestamp;
    }

    public long getAmount() {
        return mAmount;
    }

    public long getFee() {
        return mFee;
    }

    public TransactionStatus getStatus() {
        return mStatus;
    }

    public int getHeight() {
        return mHeight;
    }

    public int getDepth() {
        return mDepth;
    }

    public ArrayList<OutputRecord> getReceivedOutputs() {
        return mReceivedOutputs;
    }

    public ArrayList<OutputRecord> getSentOutputs() {
        return mSentOutputs;
    }

    @Override
    public String toString() {
        return "TransactionRecord{" +
                "mTxHash=" + mTxHash +
                "," + "mTimestamp=" + mTimestamp +
                "," + "mAmount=" + mAmount +
                "," + "mFee=" + mFee +
                "," + "mStatus=" + mStatus +
                "," + "mHeight=" + mHeight +
                "," + "mDepth=" + mDepth +
                "," + "mReceivedOutputs=" + mReceivedOutputs +
                "," + "mSentOutputs=" + mSentOutputs +
        "}";
    }


    public static final android.os.Parcelable.Creator<TransactionRecord> CREATOR
        = new android.os.Parcelable.Creator<TransactionRecord>() {
        @Override
        public TransactionRecord createFromParcel(android.os.Parcel in) {
            return new TransactionRecord(in);
        }

        @Override
        public TransactionRecord[] newArray(int size) {
            return new TransactionRecord[size];
        }
    };

    public TransactionRecord(android.os.Parcel in) {
        this.mTxHash = in.readString();
        this.mTimestamp = in.readLong();
        this.mAmount = in.readLong();
        this.mFee = in.readLong();
        this.mStatus = TransactionStatus.values()[in.readInt()];
        this.mHeight = in.readInt();
        this.mDepth = in.readInt();
        this.mReceivedOutputs = new ArrayList<OutputRecord>();
        in.readList(this.mReceivedOutputs, getClass().getClassLoader());
        this.mSentOutputs = new ArrayList<OutputRecord>();
        in.readList(this.mSentOutputs, getClass().getClassLoader());
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(android.os.Parcel out, int flags) {
        out.writeString(this.mTxHash);
        out.writeLong(this.mTimestamp);
        out.writeLong(this.mAmount);
        out.writeLong(this.mFee);
        out.writeInt(this.mStatus.ordinal());
        out.writeInt(this.mHeight);
        out.writeInt(this.mDepth);
        out.writeList(this.mReceivedOutputs);
        out.writeList(this.mSentOutputs);
    }

}
