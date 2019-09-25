// Copyright (c) 2015-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

// Portions of this code have been copied from PPCoin
// These portions are Copyright (c) 2011-2013 The PPCoin developers
// and are Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GULDEN_AUTO_CHECKPOINT_H
#define  GULDEN_AUTO_CHECKPOINT_H

#include <map>
#include "net.h"
#include "util.h"
#include "chainparams.h"
#include "consensus/params.h"
#include "consensus/validation.h"


class uint256;
class CBlockIndex;
class CSyncCheckpoint;

namespace Checkpoints
{
	extern uint256 hashSyncCheckpoint;
	extern uint256 hashPendingCheckpoint;
	extern CSyncCheckpoint checkpointMessage;
	extern CSyncCheckpoint checkpointMessagePending;
	extern uint256 hashInvalidCheckpoint;
	extern CCriticalSection cs_hashSyncCheckpoint;
	extern CBlockIndex* GetLastSyncCheckpoint();
	extern bool ValidateSyncCheckpoint(uint256 hashCheckpoint);
	extern bool ReadCheckpointPubKey(std::string& strPubKey);
	extern bool WriteCheckpointPubKey(std::string& strPubKey);
	extern bool ReadSyncCheckpoint(uint256& hashCheckpoint);
	extern bool WriteSyncCheckpoint(const uint256& hashCheckpoint);
	extern bool AcceptPendingSyncCheckpoint(const CChainParams& chainparams);
	extern uint256 AutoSelectSyncCheckpoint();
	extern bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev);
	extern bool IsSecuredBySyncCheckpoint(const uint256& hashBlock);
	extern bool WantedByPendingSyncCheckpoint(uint256 hashBlock);
	extern bool ResetSyncCheckpoint(const CChainParams& chainparams);
	extern void AskForPendingSyncCheckpoint(CNode* pfrom);
	extern bool SetCheckpointPrivKey(std::string strPrivKey);
	extern bool SendSyncCheckpoint(uint256 hashCheckpoint, const CChainParams& chainparams);
	extern bool SendCheckpointInvalidate(uint256 hashInvalidate, const CChainParams& chainparams);
	extern bool IsSyncCheckpointTooOld(unsigned int nSeconds);}

class CUnsignedSyncCheckpoint
{
public:
	int nVersion;
	uint256 hashCheckpoint;      // checkpoint block

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action)
	{
		READWRITE(this->nVersion);
		nVersion = this->nVersion;
		READWRITE(hashCheckpoint);
	}
	void SetNull();
	std::string ToString() const;
	void print() const;
};

class CSyncCheckpoint : public CUnsignedSyncCheckpoint
{
public:
	static const std::string strMasterPubKey;
	static const std::string strMasterPubKeyTestnet;
	static const std::string strMasterPubKeyOld;
	static std::string strMasterPrivKey;

	std::vector<unsigned char> vchMsg;
	std::vector<unsigned char> vchSig;

	CSyncCheckpoint();
	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action)
	{
		READWRITECOMPACTSIZEVECTOR(vchMsg);
		READWRITECOMPACTSIZEVECTOR(vchSig);
	}
	void SetNull();
	bool IsNull() const;
	uint256 GetHash() const;
	bool RelayTo(CNode* pnode) const;
	bool CheckSignature(CNode* pfrom);
	bool ProcessSyncCheckpoint(CNode* pfrom, const CChainParams& chainparams);
};

class CUnsignedSyncCheckpointInvalidate
{
public:
	int nVersion;
	uint256 hashInvalidate;      // checkpoint block

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action)
	{
		READWRITE(this->nVersion);
		nVersion = this->nVersion;
		READWRITE(hashInvalidate);
	}
	void SetNull();
	std::string ToString() const;
	void print() const;
};

class CSyncCheckpointInvalidate : public CUnsignedSyncCheckpointInvalidate
{
public:
	static const std::string strMasterPubKey;
	static const std::string strMasterPubKeyTestnet;
	static const std::string strMasterPubKeyOld;
	static std::string strMasterPrivKey;

	std::vector<unsigned char> vchMsg;
	std::vector<unsigned char> vchSig;

	CSyncCheckpointInvalidate();
	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action)
	{
		READWRITECOMPACTSIZEVECTOR(vchMsg);
		READWRITECOMPACTSIZEVECTOR(vchSig);
	}
	void SetNull();
	bool IsNull() const;
	uint256 GetHash() const;
	bool RelayTo(CNode* pnode) const;
	bool CheckSignature(CNode* pfrom);
	bool Process(CNode* pfrom, const CChainParams& chainparams);
};
#endif
