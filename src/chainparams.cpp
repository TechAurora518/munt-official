// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2016-2020 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@gmx.com)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "chainparams.h"
#include "consensus/merkle.h"
#include "crypto/hash/sigma/sigma.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <cstdio>
#include "chainparamsseeds.h"
#include <validation/witnessvalidation.h>

static CBlock CreateGenesisBlock(const std::vector<unsigned char>& timestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew(1);
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << timestamp;
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].output.scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis.vtx.begin(), genesis.vtx.end());
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const CScript genesisOutputScript = CScript() << 0x0 << OP_CHECKSIG;
    return CreateGenesisBlock(ParseHex("4f6e206a616e756172692031737420746865204475746368206c6f73742074686572652062656c6f7665642047756c64656e"), genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

CChainParams::CChainParams(): fIsOfficialTestnetV1(false) {}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 388381; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 363725; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.powLimit =  uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // Gulden: 3.5 days
        consensus.nPowTargetSpacing = 150; // Gulden: 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].type = Consensus::DEPLOYMENT_POW;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].type = Consensus::DEPLOYMENT_POW;
       
        consensus.fixedRewardReductionHeight=250001;
        consensus.pow2Phase2FirstBlockHeight=778177;
        consensus.pow2Phase3FirstBlockHeight=778301;
        consensus.devBlockSubsidyActivationHeight=1030001;
        consensus.pow2Phase4FirstBlockHeight=1131652;
        consensus.pow2Phase5FirstBlockHeight=1140958;
        consensus.deltaV3ActivationTime=1598443200;

        // Message start string to avoid accidental cross communication with other chains or software.
        pchMessageStart[0] = 0xfc; // 'N' + 0xb0
        pchMessageStart[1] = 0xfe; // 'L' + 0xb0
        pchMessageStart[2] = 0xf7; // 'G' + 0xb0
        pchMessageStart[3] = 0xe0; // 0xe0 (e for "echt", testnet has 0x02 as last byte)
        vAlertPubKey = ParseHex("073513ffe7147aba88d33aea4da129d8a2829c545526d5d854ab51d5778f4d0625431ba1c5a3245bdfe8736b127fdfdb488de72640727d37355c4c3a66c547efad");
        nDefaultPort = 9231;
        nPruneAfterHeight = 200000;

        genesis = CreateGenesisBlock(1009843200, 2200095, 0x1e0ffff0, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHashPoW2();
        assert(consensus.hashGenesisBlock == uint256S("0x6c5d71a461b5bff6742bb62e5be53978b8dec5103ce52d1aaab8c6a251582f92"));
        assert(genesis.hashMerkleRoot == uint256S("0x4bed0bcb3e6097445ae68d455137625bb66f0e7ba06d9db80290bf72e3d6dcf8"));

        vSeeds.push_back(CDNSSeedData("seed 0",  "seed.gulden.com", false));
        vSeeds.push_back(CDNSSeedData("seed 1",  "amsterdam.gulden.com", false));
        vSeeds.push_back(CDNSSeedData("seed 2",  "rotterdam.gulden.network", false));
        //vSeeds.push_back(CDNSSeedData("seed 3",  "seed.gulden.network"));
        //vSeeds.push_back(CDNSSeedData("seed 4",  "seed.gulden.blue"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,38);// 'G'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,98);// 'g'
        base58Prefixes[POW2_WITNESS_ADDRESS] = std::vector<unsigned char>(1,73);// 'W'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,38+128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fUseSyncCheckpoints = true;

        checkpointData = (CCheckpointData){
        {
            {      0, { uint256S("0x6c5d71a461b5bff6742bb62e5be53978b8dec5103ce52d1aaab8c6a251582f92"), 1009843200 } },
            {   5000, { uint256S("0x07ba1142fe1c0f4514b8ef029a687e45d08f4f930e14c753b8b8131df6d33a5b"), 1396776675 } },
            {  10000, { uint256S("0x25a619632ea07771156d61791245e7b3497ae987ef6be5348c41380291848974"), 1396928263 } },
            {  15000, { uint256S("0x944e0468c38392c5f32818f8f50c10aa6deb5986d85a72e9aaddfe94acc74a5c"), 1397078470 } },
            {  20000, { uint256S("0xbaf0214a1e00f2c265ac64a25c309dea2fa0fc2c56f1f40501981e1936405a38"), 1397230260 } },
            {  25000, { uint256S("0xe83e474bcad07e9b09caa5932d35c39812a485dadad3dba97ff1c2f842ab1cbf"), 1397382466 } },
            {  30000, { uint256S("0xb388498efce825de5df82c458513b11a657b3499b62be70aac36720ba661a792"), 1397535343 } },
            {  35000, { uint256S("0xe14bac6cfea31014bb057500160fb5a962e492ce16652b14fa07314fd9e523ff"), 1398287041 } },
            {  40000, { uint256S("0x7e460787e1820d608fac250d7b53404b131804eec4800e9efc80a5eca7b30349"), 1399043662 } },
            {  45000, { uint256S("0x97b4cff99eda714dbff09881e339d1159e5558486e31198affd712ca806f0b1d"), 1399803213 } },
            {  50000, { uint256S("0x5baeb5a5c3d5fefbb094623e85e3e16a1ea47875b5ffd1ff5a200e639908a059"), 1400560264 } },
            {  55000, { uint256S("0x48bcc20fe91362c87727f710aa99523cc24fd95e2d2badd7fddfe220b4824747"), 1401308606 } },
            {  60000, { uint256S("0x289c63315033285a31661ce4f26b49cdb5f25f78edb87a7ab89c2718240eaab4"), 1402055764 } },
            {  65000, { uint256S("0xea528b9c5da15983e0c070da93a8c352c3227c9c1f4e55f4c6c737a9e5a9bcad"), 1402815392 } },
            {  70000, { uint256S("0xce9267eeaca828d4152315e714b4089ada5ce91f5d31aff8b6394b3680d39d99"), 1403573297 } },
            {  75000, { uint256S("0xc6ccc28c6dbe5760ce5154f7e9cea405f6521d6b4f9ec21c988f5f7e626b451f"), 1404317484 } },
            {  80000, { uint256S("0xba0038c46553013f8ac4a6440363e0b65a2a56be4e2c1ad92af694330dd2dc44"), 1405072543 } },
            {  85000, { uint256S("0x25a78f44abf0c5ec967a77f458ad6d2ad8f9647d409206bb857e2b4bf6ac13ac"), 1405826320 } },
            {  90000, { uint256S("0x46b0f4d321a56aa18f96eff03728de63069fdf5e6309b00feee87f10f25f5003"), 1406575129 } },
            {  95000, { uint256S("0xaf68270d7fd7a60e94131c963f00632169434d6a7d1aed41143739b32e466b18"), 1407328518 } },
            { 100000, { uint256S("0x5e831ed155d05f6ac7f17635022dbc348bf73942309ac403c6f8c2990e2e0af1"), 1408078859 } },
            { 105000, { uint256S("0xea94d0eb8f10c9957b34e6edb7be406258270564f585114e70e478b7470f5685"), 1408820382 } },
            { 110000, { uint256S("0x3f87ae7d4f4c6ec94b17e82f0e0998a3a85c98a3131952b5e258da69ab9d2aeb"), 1409573417 } },
            { 115000, { uint256S("0xc6e30bd59160fbcd460f1144be7b40bc078b37347149beeb09ebd3442aa85934"), 1410324413 } },
            { 120000, { uint256S("0x38004f47dbff789f7ca48e366f89b3ff4e0f6334d6742e234f8dd166a063347b"), 1411089720 } },
            { 125000, { uint256S("0xee27d0f4b6596f302eb591072136ae196bb318d776c16625b23cc7383052b564"), 1411871556 } },
            { 130000, { uint256S("0x9fd52400fb1a461311eaa2aafc32467808189f8fa2d81d2a8cbc1dd8539446fd"), 1412655875 } },
            { 135000, { uint256S("0x573e70d731f245fd4f4e79a6d28f5dfbfe45255da70685e303625a5a82d9954d"), 1413447624 } },
            { 140000, { uint256S("0x010019afea8185a2692981fb040fd1eb8ab05479d65137849a0cf66d39bfddbb"), 1414276483 } },
            { 145000, { uint256S("0x4b6b2182ce2445e76c00e3487fbe83ee436935911599b058818d08aebea0be8c"), 1415061802 } },
            { 150000, { uint256S("0x97fdb21189d5a958d42fcb58f8d300e737a20fad91878dabdd925d11fc614013"), 1415846757 } },
            { 155000, { uint256S("0x61ca04d27282ab659c087bf8877f461e0d978f2933bd32a660c5096deb72fe4d"), 1416631222 } },
            { 160000, { uint256S("0x87e7dbf8f3cdc490fdb05c4c1ebf691bc68e566095d51713132e53584961d5c2"), 1417416595 } },
            { 165000, { uint256S("0x4b9efbb5350a2a3052b516ca9a1fa526b0efa68deec20767ba5c1c99dcac798f"), 1418202928 } },
            { 170000, { uint256S("0x91ad5405df477c70a01e9cb4ff10cc4f1d80487aa4d9e86a4867b8084a7c216f"), 1418987643 } },
            { 175000, { uint256S("0xda6aa09113ddd62d871e9aacad6131831d5841a26968f1665a9b829fd30a29e3"), 1419771515 } },
            { 180000, { uint256S("0x4bde66d4babf6e480587582ab62f09bfa11450a66f753e4ed40d7540f917476e"), 1420557596 } },
            { 185000, { uint256S("0x4d0413eaf446bf4b071f401b3b3410c1ff3653be2a2d24c9047cadb29d01e8ff"), 1421342229 } },
            { 190000, { uint256S("0x2e1e89c211c3431a0c3d2b2901f22295975d1e7f7fc1013d9732f10d93708a9a"), 1422128561 } },
            { 195000, { uint256S("0xc18c38f7fa1bdf626861c38e0b4e3b785e926ab3465e651a7b854a800cd9af56"), 1422958219 } },
            { 200000, { uint256S("0x4e80313f4eb23093a63218f3736379084d1eeae46c4343668f3cdc9c0c5ca260"), 1424526562 } },
            { 205000, { uint256S("0xd2b896c1dd16dde4d70dac46cd2157fc6ab6b7875e5932b211362758803be366"), 1426695052 } },
            { 210000, { uint256S("0x1f1b4e0ac592bcf62ae9a26c6d6e76c96f72d6f2aeb352d0d6237f32ebc66f8b"), 1429105034 } },
            { 215000, { uint256S("0xc858f6f5f922307867a640c7b1ae7f496653a9811c0cff807a81b0fbe4b18151"), 1432175570 } },
            { 220000, { uint256S("0x6b7b5585d8a1d84ac1c7312930389da5ada65fd5b25e40e74e157d909a8a0521"), 1433232658 } },
            { 225000, { uint256S("0xc9a5c5226d8f103972ffee38c31c3508189b694e0d4f93a394ccea2cac82ce49"), 1434583308 } },
            { 230000, { uint256S("0x2a1588f8f138a189d1890be022f607e9b3a00b68c2062fcc1efd3a68b4c0946e"), 1435964425 } },
            { 235000, { uint256S("0x35c62070ec88311335f347839e07aaf00205c3cc8fb5017b95aa41f1264a8694"), 1437298404 } },
            { 240000, { uint256S("0x72783fe0d761d29561f05ee43c77fdc3f59a9540b7ac1b5849f346e9474cf6ca"), 1438670438 } },
            { 245000, { uint256S("0xbcace89a7a4d69e4a8e4efb4e41bcd921a2a2892035df04e86ac63bf6b7394f5"), 1439984025 } },
            { 250000, { uint256S("0xa6635e1dbce15cfb4be7f3f464f612205dd13ba96828535000b99ce04648500d"), 1441212522 } },
            { 255000, { uint256S("0xcecee67fb8baf21d4995015f26f3933b841c7f69c8645c500783d33108c6676b"), 1442094027 } },
            { 260000, { uint256S("0x42c2254ffd8be411386b9089fec985fe3a06d5fc386ff0bd494b5a3aa292f107"), 1443116050 } },
            { 265000, { uint256S("0x9fee70b9671b30396c80559500e146924cafafc16f673772b9bbb38d364c08aa"), 1444174457 } },
            { 270000, { uint256S("0xfa3c642f0c44da06892edaace1e2da0e66f62574401e1a8b7482264a1bbc59ba"), 1445327681 } },
            { 275000, { uint256S("0x3f602d877314f330da40bfda7cc41198eec3f1ebd64ecb8defa30efe62fcb8d0"), 1446402538 } },
            { 280000, { uint256S("0x2f33b4d16cceef28540a62c7fe061b465cdea93dd1a32b787152fd11c719b4ed"), 1447507603 } },
            { 285000, { uint256S("0x58eb0593b7ba41d06f4edc58a4ef6b1029ff6962ea3ad87a286bbe72251a4120"), 1448529827 } },
            { 290000, { uint256S("0x57622177fa40aae7dfdd6636c0ccd436e0f95c09714126fcf809915ca40cb7e7"), 1449675808 } },
            { 295000, { uint256S("0xb2d3a1d1c622683524d15216e3ac137bf43587116d4494fe20c08d82c4211674"), 1450705596 } },
            { 300000, { uint256S("0xf0f99e78c90d20ac4e376ffd1a7e8a89cd1bd152ad1a40f7be31bbf8e0b492c5"), 1451816529 } },
            { 305000, { uint256S("0x01fb036d4a39e9476e63c6c681b29ae9fb96dccdc8e548957ab736b6a5847cb5"), 1452856123 } },
            { 310000, { uint256S("0x6a77548fcb36d46053e883eb9814cd70f165f2dbc0325aab2261410666361dd4"), 1453885717 } },
            { 315000, { uint256S("0x27ebdefc75e8ba0611b4397e8d58ef7356bfcf7169c4824930f181289f587cf5"), 1455011314 } },
            { 320000, { uint256S("0xffd2c9c93b15e32875664cb80ad974b8c7847e5c6eb5d970a9fad6df126c4366"), 1456152463 } },
            { 325000, { uint256S("0xa00bccd7a68495771f03856633786a16d3106b38adeafc4d610918b5b118ec9e"), 1457341408 } },
            { 330000, { uint256S("0x23f895ebde2aac83769aa475f063cc2b96bc6f4a8ffa9e7f473bf520707f8858"), 1458367876 } },
            { 335000, { uint256S("0xa5a2446a295cebb6eeec80e8d844b198308aad99d86c70208783575926ecddf2"), 1459407891 } },
            { 340000, { uint256S("0xe69c9f1a872760c26b222f9762ccfa8924a9aae419334b6aabccef7e5a77e07c"), 1460363950 } },
            { 345000, { uint256S("0x417b9800461461bc1346e1659726b424a5b81d273df3af4972d4487506a56412"), 1461334497 } },
            { 350000, { uint256S("0xa4c92744d47ada905f5cacc7ee91d86f0e646d52d5d8cafdab5d288490002196"), 1462316267 } },
            { 355000, { uint256S("0x0ecfa10c0e7b43669f72483285c0bbab1ff333af0574f7415c774e3334f6b9dc"), 1463405995 } },
            { 360000, { uint256S("0xdfc2eeae2ff681fdc1426a4b6da0c0a3d19d8409668515dae8834d98be93e6e7"), 1464432780 } },
            { 365000, { uint256S("0x736e3eec11888374e93ba5962d1203d3ead84b2e90152e97199f1e26d7fb76c3"), 1465519318 } },
            { 370000, { uint256S("0xfc8a004796035eedc19f43ad79ea153f2f10ceb3557d7349e90c95f1f2c55364"), 1466632020 } },
            { 375000, { uint256S("0xa1c3e45a3b4bbf823a35433f604d33c89749a3950a793becc90cddbecd03409c"), 1467984054 } },
            { 380000, { uint256S("0x67e151608de36d87336931ddb0ec381f24348663a68e7032f0c768fb1cf054e2"), 1469127714 } },
            { 385000, { uint256S("0x379d523bc9fd29b7cb260b5161dbde9cd07d1a57cf4600ee689e31d643e48004"), 1470263618 } },
            { 390000, { uint256S("0xf18a084d2d73fa07e2c0e2cdfe8c2ddff1b01c721aa2a4b63687f35b79ff2247"), 1471554451 } },
            { 395000, { uint256S("0xe3058781206326a193b851a80489b30fbf660c3cca62816a0f57f993cb1cd4c3"), 1472708456 } },
            { 400000, { uint256S("0xe4b072d2861b8041f42dcf2e8f5d1caaaff36bc952518e99f6d3fac89e1e1133"), 1473722382 } },
            { 405000, { uint256S("0x63666839cf8138a012b4bf6c29c54246c9ce32875a349cbd09847e37ac8602d1"), 1474733451 } },
            { 410000, { uint256S("0x6196ce0df794d8b0456ea7b1c2dad12ae312aa3ad5dbb6ec59028f4770e37e37"), 1475808812 } },
            { 415000, { uint256S("0x34d62d2bff54f6e86209d34162538e33cf74c9becbd4e682c568f945a2271bc3"), 1476973453 } },
            { 420000, { uint256S("0xe77dda63bd507c5925720695c480feee3ca85cb6fd62ccbafd319e7dd919a863"), 1478078687 } },
            { 425000, { uint256S("0x0dd5aa1302943e7b4fff1963ce1c32a1aa3db15d652515a3ca8bf4aad84952d7"), 1479092395 } },
            { 430000, { uint256S("0x4296798738369477be63e58ffcdf6eeb1dcf3c61f7e7af08f5712317c079ff7f"), 1480146233 } },
            { 435000, { uint256S("0x0d7bacd32a25637740bdad6b8d21d635414be1766527053fa2161c2cf3ac7fbc"), 1481251184 } },
            { 440000, { uint256S("0x2b06689877981bd04ecf34d281ab96af1d7decb1438e4df40a2236562ca5b09d"), 1482179411 } },
            { 445000, { uint256S("0x55669afdefdf89251fbac44bd1eb1788b14d8f2dd9aa5887486229180529c6b6"), 1482927930 } },
            { 450000, { uint256S("0xfa4f46c846b053104ce5956578d72f9a5fa87c4ae49a6450e5d66c4fd37d6d66"), 1483665997 } },
            { 455000, { uint256S("0x46bc63bc67a0baaf1fecc1cf2e435458587180212450af24a37df7b0b02002b2"), 1484406588 } },
            { 460000, { uint256S("0x7da617ca15be1f39dee7294d26a95f192fbb72e3cee8ca804e9d1d27f661fb83"), 1485145394 } },
            { 465000, { uint256S("0x3032195da4adb3209d55ba881831dec748695c4206cd18052fdcf046d142bb49"), 1485889227 } },
            { 470000, { uint256S("0x0656688a2c1db2cf5a3c8412ad05d2d1784e1ee630444f007c33f83765bad579"), 1486626192 } },
            { 475000, { uint256S("0xa730a89a11332ee0133c458c5feccbda857ace9572ab5a048701cccb0239cf4c"), 1487361080 } },
            { 480000, { uint256S("0x4616acf55fe787c99843ec3d32d881f0b70a9d03490b00a22e2eb03ef6e174fb"), 1488099040 } },
            { 485000, { uint256S("0xa6f242ae9811b724d7f446e9482991cde00da6c4c69db25b570ee19818eea4a3"), 1488834434 } },
            { 490000, { uint256S("0x9bea668c533839f89941ff66467a7f9690116d49121b69bebdcecbf7c137d94f"), 1489569017 } },
            { 495000, { uint256S("0xa6bd3510860377e0f998eae41b901b13f41686dd39bc3116d155819616d5b4bf"), 1490302633 } },
            { 500000, { uint256S("0xd30e19c8b8c567b23c09fc022b4f5ca8014a8cb3c1504782e9a68af349757afa"), 1491057581 } },
            { 505000, { uint256S("0x56bce924eb7613b6fd4ac859a06a13f7643817d6a593d19951ab293182a021cb"), 1491810603 } },
            { 510000, { uint256S("0x17b078729613bf93c40d605ec1f763e0ba9bd23eb0902c3e74afba8f291aa834"), 1492558829 } },
            { 515000, { uint256S("0xbc1264dd2417fede0c18d18f10cb2b96bea78eb5e38f3b90b0069cdcf0b44c84"), 1493304469 } },
            { 520000, { uint256S("0x5627419fa4a289e54c18b5b74c0692248b2b7e6c5612cf15717afbdee7a8c525"), 1494051639 } },
            { 525000, { uint256S("0x1d3f4de346830365474b24f7b9dd7b332215dbe0e9e175c70df040446e2c4185"), 1494799318 } },
            { 530000, { uint256S("0x1f283d04cde433cd21fc37bce84e24ab5bd91a162c9cbd65917ab61b64faa4c8"), 1495545207 } },
            { 535000, { uint256S("0x53ee7fa548f92b54ff29f8015d647f4c2525f91f2eb624f12ffec932bc0af294"), 1496295697 } },
            { 540000, { uint256S("0x62e5636387aaa74f6c48ba812710481620efdc35e9fd5eb1bdc895e64d110a3c"), 1497037983 } },
            { 545000, { uint256S("0xecd86540dd3faebbf860f35a9fa92e1145c3470e998dcad3716fff241d0e25fb"), 1497785963 } },
            { 550000, { uint256S("0x3d7fc31f64905e2e6559298b826b914a3716ae160caea485ae46230890a4144d"), 1498527420 } },
            { 555000, { uint256S("0x8d17e332f8a2121434fb6f7c650c6cae7f575b2e4f8be6566d8d08ba09bb152c"), 1499259716 } },
            { 560000, { uint256S("0xbd60ce2fee93534e73d4b339c986e0c251a07902caad18de54049daf282d6560"), 1499991286 } },
            { 565000, { uint256S("0x617df1fa71f67eeb4ab9cf36e99c24602716f21a717e404838a8f01690fb4994"), 1500728771 } },
            { 570000, { uint256S("0xc2a212411ae8d01c113161ee4e261890186dc98aaf0e1fbc202d5af7cf85ff1d"), 1501459330 } },
            { 575000, { uint256S("0x728d0d7edd0c1fcc0fa0f80bfbf8c06e2afb9a8ba0358fa434d3114a982003e6"), 1502185812 } },
            { 580000, { uint256S("0xa92b260332276147a552e2350a95a04fe7fca1164eb066e7f02dca3ae7a68b3e"), 1502918516 } },
            { 585000, { uint256S("0x1fa14d4a816fb35386873f49ff822d1cf225989ed45332ce31d21575638e8c7c"), 1503650754 } },
            { 590000, { uint256S("0x9b5e5464538123fd0542735875bc4502246a58aff68e0fd9c3ab0d2f2977faea"), 1504390043 } },
            { 595000, { uint256S("0xf80a3148c3f333ae17d6e7e84229cde85b63f2bd93d73aa3fa84bb17ffe1f38a"), 1505128175 } },
            { 600000, { uint256S("0xd6f49c4bf3b3a0e82392809b8925c485a518acdeacf8bdd9a9326e4c68456d6b"), 1505870868 } },
            { 605000, { uint256S("0x44562c273753696c5d0bdf2995445ee25a514d36f61644aa65ddc48c739979cf"), 1506603940 } },
            { 610000, { uint256S("0xbbafa1669017ee72cbb24092d16cf42fa667ad3dd178d60391ecba22ef3e51fa"), 1507344623 } },
            { 615000, { uint256S("0xccda751664a400dc813bd4ad91671a2c7b82f75326eb336060675f66315ff57b"), 1508088529 } },
            { 620000, { uint256S("0x288777a4c134b9582fa81f4bb342fcadd2c58bd2cba9c7c61fb66fabc38b7b66"), 1508829713 } },
            { 625000, { uint256S("0x0d067e2b98621356f67c0d969c8dbccd62a6bea757a006a6640e42becfe54740"), 1509573613 } },
            { 630000, { uint256S("0xa6ba7f5eb8aae5d636f7057ef101452015be4831f08dfdbf9e4870b2794a97f6"), 1510318993 } },
            { 635000, { uint256S("0x880773d0d7085341798b7b7411506eef6e397808ad3265ea090850db7f2636c6"), 1511062514 } },
            { 640000, { uint256S("0xeba2fff77eee4bfbd49f8bce8d3ac125f8894d817be8a10524d1f592ff0c171d"), 1511809837 } },
            { 645000, { uint256S("0x760ee56cbc369f217832d72ac6b618646113a6d5aaddb86a15fef7aaa2b143f8"), 1512550170 } },
            { 650000, { uint256S("0x0fcd45353b44036efdabed53931b6a49db8f44fa91ede6be64189bccc5f2af53"), 1513296093 } },
            { 655000, { uint256S("0x4ed2f0a00af3fe478395c78fa56e14599c8e83b336d023412e8121716f4c77e3"), 1514033954 } },
            { 660000, { uint256S("0x8fd5e894f1a3b1662fd68ccbe8df3f6cf7bf9daf7d01b2bc0b58d98be05bb2f7"), 1514768121 } },
            { 665000, { uint256S("0x83a68b3a7ff7c9418fecf987e4f641ac1e941128df3e38f094056bfa555b514e"), 1515497950 } },
            { 670000, { uint256S("0x9de98c27385e461e105dc7821ad2b8d534b20400faf3c13367c96ca5bf2ae3b7"), 1516229841 } },
            { 675000, { uint256S("0xed520161b5954c1a800497fe334e320bc481f2b103f78e8319a0db6cbbd2dcb1"), 1516960413 } },
            { 680000, { uint256S("0xa7efadd6fd29ea5d74a36dad4ada8be0be7562ffb3a023629ca70fd3d3a9d852"), 1517694897 } },
            { 685000, { uint256S("0xeb30f10408b7b0e9732f0e93f3609e890f19cf598e6d79ca58fa0a5927e0a8ae"), 1518425478 } },
            { 690000, { uint256S("0x632da89ac12d274dfabb7912fa3ad5a95a4664213d5ee96532195a44999b4944"), 1519157224 } },
            { 695000, { uint256S("0x0172de09b4224dbb4235be95bfda6ad8bf9e6546da06cdebc1ab287255fbbfab"), 1519887659 } },
            { 700000, { uint256S("0x7cdbb7bef28741aa682570703ca03cd77a6524011aed588fd0aabe5f0038f124"), 1520617979 } },
            { 705000, { uint256S("0x071156c7d447e7a5940d2b2bf75935a542b953c6f075264563b0723e70ecfbcf"), 1521353177 } },
            { 710000, { uint256S("0x9d21c021202747451b5caf77477bee39b0f7307b058e7551ef55c7955e68de9b"), 1522089013 } },
            { 715000, { uint256S("0x185c3c38a6a33b4c6803d58d67f8435cec67a2d4bcb61ffa34acc390503cb04a"), 1522822445 } },
            { 720000, { uint256S("0xcefa30dadbae53f7e1e3400489ad2b55c9490a6ce91a9d74e5702c09f37986de"), 1523551635 } },
            { 725000, { uint256S("0xef7425f36695916b23a14e9a23572fda9ca5ceb39d7029274889b1700cd65436"), 1524283360 } },
            { 730000, { uint256S("0x25e40efb33eb713784c86fee4013c52efafed7a0ad5aa0b4a4266e73cd619289"), 1525012542 } },
            { 735000, { uint256S("0x17d9585d6c2d4312a1ce583611d7193759a8ec29301c18b090693fa5f80bcbd9"), 1525741792 } },
            { 740000, { uint256S("0xba1d907e5b328323778e88a76730ee3ae38fecc432bb288c0eadec26fa4f6544"), 1526472312 } },
            { 745000, { uint256S("0xcd2111fa34197e4d8eeaa5ab014bde09e2f823bfaac2ff823367f04710a2d1c3"), 1527202765 } },
            { 750000, { uint256S("0x22330a217c970fce0ac14f954793f0116df6931b1fd9f2c9e469884a71ef4d96"), 1527932637 } },
            { 755000, { uint256S("0xa678f0896638028ecadd4d17fa139c834a4a158a73a0e8d9377e38a8f2d2ae96"), 1528666208 } },
            { 760000, { uint256S("0x6c311893597eb070949de2929b655c364cff0fe5486aaa5e6de4ae6db41e36f2"), 1529400789 } },
            { 765000, { uint256S("0x6bad0bc8dea23fea8a6cf7755436dc6ff765a5bdfa01a10bfffd0a21e24868e5"), 1530130251 } },
            { 770000, { uint256S("0xe55b1550d0f06f5ba51cb101b23fa181d646f3bfbcd512ba3466ebff29393194"), 1530859532 } },
            { 775000, { uint256S("0xab6660b98b64cb58f0c7595dd46a749ebe76fdfe20996c84defca6573df2c2b1"), 1531598367 } },
            { 780000, { uint256S("0xddca858d7e0bf263ac4a2641831b2e8578b1ef8418a0621f9f6263666737539b"), 1532400008 } },
            { 785000, { uint256S("0x318762ff8a035075584065c1101dae5c7cac5b5d3c26a666027c9a5e6f3aee49"), 1533172593 } },
            { 790000, { uint256S("0xd148b595af81bcfb5611b93aa75814cb5558e8774bbe1adc068ea7696bbbdbaf"), 1533924977 } },
            { 795000, { uint256S("0xc12484777b90ba6274f3f618e7ad24b002aa09d89f5ed50154db1097d6b5e6e7"), 1534668951 } },
            { 800000, { uint256S("0x6e5d2d9c616921061743fa3206bf26f20dea7fda1abec6309f453e4b9bdf84ac"), 1535429535 } },
            { 805000, { uint256S("0x02bdbc0a65044ca98855bfe2bc91690029537f265fa52bae9d0da9936c4934e8"), 1536190924 } },
            { 810000, { uint256S("0xdf61ac9e368f000411e0a5a05617c27f5b51c989d096919c4fe1dc27d2e6ef28"), 1536954107 } },
            { 815000, { uint256S("0xd40c7c94d9247688cff0d68261ae1589745eacfdcc65bde576d14e8f0eb8970c"), 1537712836 } },
            { 820000, { uint256S("0xe70b21679f7c1a33e34a60abc90f7fa11290ad34f261f2976d6096b3613df3ae"), 1538471583 } },
            { 825000, { uint256S("0xe60fa4bcc9594d03ca687101cab8a7bd2ee52c14104d04d5194fc85c69f17064"), 1539232375 } },
            { 830000, { uint256S("0x6b19deb12fd14da4f62233027410d447925f169fd5a76c932ced28f022edaa74"), 1539986915 } },
            { 835000, { uint256S("0x382fe6505b3dbde4604d67930b1a4f0b2b77452bd776046e30da6a31138227fa"), 1540750990 } },
            { 840000, { uint256S("0xca680727ba57555a1d80add71a81d63f57592612237f370b4adc96d906b508c3"), 1541516632 } },
            { 845000, { uint256S("0xc20ec608f7de7a3c44fcc4df3c96a3afb27e176673f80881da96f9d0a407bb35"), 1542276904 } },
            { 850000, { uint256S("0x9d2ecf60d1e0cca1510eb211ed3fd2e049a467ccfdf13d10b02c90ea8e92e6b9"), 1543034510 } },
            { 855000, { uint256S("0xdcfdea952f6fb298692dfc32135ade0084707c0491673075a630a78a40f8eb55"), 1543797247 } },
            { 860000, { uint256S("0xbe7ba1721a7b5975f7ca41fb0a439ee2c8be287852d81583f58acf7476338511"), 1544567932 } },
            { 865000, { uint256S("0xf99f37ca66473eb3449e058cd4f922aca0ef402ce43b305af56ef6f8f81afeb7"), 1545338530 } },
            { 870000, { uint256S("0x0b20b92f4a99e8daf371ebd8842e87e19bd027e630232d804a9265bc74fa3154"), 1546115851 } },
            { 875000, { uint256S("0x6f20cd84b85f38d9885c7abfd27ab64e848e1c9d59d20087928d73be8d332856"), 1546883119 } },
            { 880000, { uint256S("0xe0afba09a075054eb901cc4fbacda150625ecad21e4c43c5b3ce59f8c5195160"), 1547662599 } },
            { 885000, { uint256S("0xa3ff2a300d41777d8c5a1f5220a185fded836dc221d0a0d625c0628eb19c15a2"), 1548441034 } },
            { 890000, { uint256S("0x0a53aa18822f68dfc2fc5a7f5999e5b31320a6ea1a9955287faf358f7dc69546"), 1549219818 } },
            { 895000, { uint256S("0x6a448ebc37915c1847d99c043a5948091d761221da9dea068d377d4dbd85a61d"), 1550006783 } },
            { 900000, { uint256S("0x7fc71149acd4dd79f27e7f73b33170d1eb35b132fdec54d84551a1b677b41f50"), 1550803236 } },
            { 905000, { uint256S("0xeaa2d4441498ab892dc8a2e112bbe8261c4555bc665b23545365b2ba71605a4c"), 1551600103 } },
            { 910000, { uint256S("0xb19ca1b1ea65fec4bb5eac8e548e29327281973a2ae0e4a4d307500a88bc7ee1"), 1552394784 } },
            { 915000, { uint256S("0x7e552505c649392fd80b63a6ecbb3c76add9edbcb8adeff019a38e59ed8a96bb"), 1553182683 } },
            { 920000, { uint256S("0x4399d4087d11da194b8385c07dd48cc1246a0ae0be66d9708d8f6f1b4a5b1a4a"), 1553976785 } },
            { 925000, { uint256S("0xc4ffcc0bff959d3bdcc4031672dc3d46d1c4964e51164387ad3d72ac67ea06b8"), 1554773170 } },
            { 930000, { uint256S("0xa0108826cc23b4717b3d3af82c9aa81c94b5382c9871bfdb2ce8ad0182d4878f"), 1555573024 } },
            { 935000, { uint256S("0x751db5ba72ab5c1f16e9f022716849dc0b87b8b132bcf4bb0363283869faf26e"), 1556361428 } },
            { 940000, { uint256S("0x03c175f4910b7779c2c6904f6aab59d4aeaff888b772830dc41973db47ea1242"), 1557134813 } },
            { 945000, { uint256S("0x7aa4c67348fbbd6435188b95fd7208269e8a803bc8d9df2bda36c6d317419675"), 1557925912 } },
            { 950000, { uint256S("0x781f3d6fde2b93c9e739016a0c4606769a64d834711262769d79226f3b8a4894"), 1558735021 } },
            { 955000, { uint256S("0x807b9d6a9eed15702bbfcb3c2748a50c7c03e34346476d8f2a7ba63bda8b813f"), 1559547345 } },
            { 960000, { uint256S("0xab00c23dbec9661f3e16330e78d22d2aed2df8e00bb3f2ac5997bd6c302e72e9"), 1560360440 } },
            { 965000, { uint256S("0xea173786128a536c7b9d2e5a40e5183674e52ffc9e4ce59dfbad30daba37627f"), 1561162218 } },
            { 970000, { uint256S("0xf08289e2f82fe8a07865db96824b3a88fff7290bc418f690784cb7243e2cdaac"), 1561972044 } },
            { 975000, { uint256S("0x570071efb5f68bb16f14af8ad9150d67b5d9940243aa1ca60dc0697f91881318"), 1562776125 } },
            { 980000, { uint256S("0xc7d52ea351c8def62d52f3f102d1b68d1d420a6b62a2cb6cacb1a57ccab447e0"), 1563573643 } },
            { 985000, { uint256S("0xa107e2c619a693015d5cd897ff181ddadedb4ebe09d9b6d774166386f5f765f6"), 1564371268 } },
            { 990000, { uint256S("0xe69a5eb05ec6c09019275eb57c9d95cb955128024592cba5fb7ffc5e82c3b736"), 1565184825 } },
            { 995000, { uint256S("0xabfefe12298e0f690767e341b2092517afd6745116ea2cd03a5c292a73668f77"), 1566037355 } },
            { 1000000, { uint256S("0x738a101e6024f9f4715793dbf684a0aa6328526fca068254ad6a56191acb5be2"), 1566810491 } },
            { 1005000, { uint256S("0x90d54954a253d456c1815982adc19af4161d3ed18403a9e8bc80d0943165ad58"), 1567588577 } },
            { 1010000, { uint256S("0x82b509dfce9edc470801e725e701530720e6badbbdfdf472b3c9379c0545f3e7"), 1568363651 } },
            { 1015000, { uint256S("0x6499eda9640f312cb9959df4f41182829d9b8ca271566cb4b232f490e9cdda02"), 1569129744 } },
            { 1020000, { uint256S("0x2a923cbf76079c29f2c181842531f903bb59bbd666dab4e048daa46d7548cdae"), 1569894326 } },
            { 1025000, { uint256S("0x46e4629e2a9d47c0cacdbd1ebfe1b337dd626263b8d18a297d707891927a21bb"), 1570665822 } },
            { 1030000, { uint256S("0x2f0bb472a7b73e1c7d68e30b19bd1f7588f29abd7355c0ff6be8f7c7a139ba7c"), 1571511759 } },
            { 1035000, { uint256S("0xf61c183cb5a0f95cd088c9c55234b577954ae1456a7acd2aea6433daca3abd6b"), 1572244451 } },
            { 1040000, { uint256S("0xdd73c2ea83c105a92538f5b6048fb7aa92b4c6c81124e4902aa958488d685d6f"), 1572975627 } },
            { 1045000, { uint256S("0x6b5dfd02c66cea52b8d6f07adac8d6ec87149680975159204e806674ca3b9671"), 1573699139 } },
            { 1050000, { uint256S("0xc51e7082a3281c8e8cfd8276207790067570df6d13103a0fd19bafef1517490f"), 1574423442 } },
            { 1055000, { uint256S("0xd1f48d9609840a608f9dfa0c2659a44bddc679ef9b46562bd818576b50ca0082"), 1575144908 } },
            { 1060000, { uint256S("0x8d0bf241dd3cb41ad2db731940344c7407a94629aacb9f026dcd94f5ce2ef714"), 1575868859 } },
            { 1065000, { uint256S("0x290c1653a28b44f97b4855a32f467de1886fd12f1cd85e386b5f8850a31d4e57"), 1576592228 } },
            { 1070000, { uint256S("0xa1a5dfe6ed73e8d32991ec1a0ed13268cc088e8dfefa01a51798ddc8a82c7951"), 1577315316 } },
            { 1075000, { uint256S("0xb4901b8546f0a594e44902e5ebefa20734783c13ef7ee89a75ad380c0ddc256b"), 1578035849 } },
            { 1080000, { uint256S("0x09e8ac6b33b48cf8701a8e84eea363c258d9cd0097f16a7f11316211b0f4e2ad"), 1578760843 } },
            { 1085000, { uint256S("0xb9c78d02108c55f033fc403d12d58741485160a8513f8e4a856d3b0f08d1322e"), 1579483986 } },
            { 1090000, { uint256S("0x062d9fbb5fc3328bd5ee52a556fe04e99a79b06ee354d8d13b92ef9ce098fcdc"), 1580197107 } },
            { 1095000, { uint256S("0xfaff013edffee9ea47d72bca92108725111c599734e2f664afe053f0f4f93186"), 1580898910 } },
            { 1100000, { uint256S("0x18b8b018d783b030e06dd9896689f6a140dce791795121ab226183011a39316e"), 1581596575 } },
            { 1105000, { uint256S("0x0ddf02d575ad50e237f359492e0f00bef7a86cd2336bfc932133e8b57a0f07d4"), 1582293978 } },
            { 1110000, { uint256S("0xeaa096c7a263ea485c8be90d26de2cf4b22a8f50252dc2a1c080f54213acf0b9"), 1582991681 } },
            { 1115000, { uint256S("0x9b6d256fbb6048c997a179f941bcdbe1b1b2aba9cd6ef6e522cef714f7d6de62"), 1583691830 } },
            { 1120000, { uint256S("0xbcc3bcb3213bc9f88a74bda5b9f30044da262733d2678bc10811c987b17c71b7"), 1584388403 } },
            { 1125000, { uint256S("0xd3d0d507764229ef0c676c87a5df6f1963f6dfa2845330ef94955de0ecc60271"), 1585084828 } },
            { 1130000, { uint256S("0x3a4c11d187451f340d928bf487e5d942d11559cb38f889cf69f05510ae8ff299"), 1585791393 } },
            { 1135000, { uint256S("0xbc68728519d04280048031d79e29e7f8595e68641349547491902b4d219d5f1a"), 1586491711 } },
            { 1139096, { uint256S("0xc166ba902a06d4313b2a6ce7b15120a298b6202981f61975043cdc0e69709851"), 1587068148 } },
            { 1140248, { uint256S("0xb15bbeb259bfe17cdb1ff8ba5ae20ba47ad733bddfb4ef7909cc63bd0221e6b1"), 1587231240 } },
        }
        };

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0xb15bbeb259bfe17cdb1ff8ba5ae20ba47ad733bddfb4ef7909cc63bd0221e6b1");
        
        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000013805beeac7c80f35");
        
        chainTxData = ChainTxData{
            1587231240, // * UNIX timestamp of last checkpoint block
            2892631,    // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.1         // * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.BIP34Height = 21111;
        consensus.BIP34Hash = uint256S("0x0000000023b3a96d3484e5abb3755c413e7d41500f8e2a5c3f0dd01299cd8ef8");
        consensus.BIP65Height = 581885; // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP66Height = 330776; // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.powLimit =  uint256S("0x003fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks

        std::string sTestnetParams = GetArg("-testnet", "");
        if (!sTestnetParams.empty())
        {
            assert(sTestnetParams.find(":")!=std::string::npos);
            assert(sTestnetParams[0] == 'S' || sTestnetParams[0] == 'C');

            int targetInterval = atoi(sTestnetParams.substr(sTestnetParams.find(":")+1));
            int64_t seedTimestamp = atoi64(sTestnetParams.substr(1,sTestnetParams.find(":")));

            defaultSigmaSettings.activationDate = seedTimestamp+300;
            if (sTestnetParams == "S1596003003:60")
            {
                fIsOfficialTestnetV1 = true;
            }

            consensus.nPowTargetSpacing = targetInterval;
            consensus.fPowAllowMinDifficultyBlocks = false;
            consensus.fPowNoRetargeting = false;
            consensus.nRuleChangeActivationThreshold = 15; // 75% for testchains
            consensus.nMinerConfirmationWindow = 20; // nPowTargetTimespan / nPowTargetSpacing

            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
            consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].type = Consensus::DEPLOYMENT_POW;

            // Deployment of BIP68, BIP112, and BIP113.
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
            consensus.vDeployments[Consensus::DEPLOYMENT_CSV].type = Consensus::DEPLOYMENT_POW;

            // The best chain should have at least this much work.
            consensus.nMinimumChainWork = uint256S("");

            // By default assume that the signatures in ancestors of this block are valid.
            consensus.defaultAssumeValid = uint256S("");

            consensus.fixedRewardReductionHeight=1;
            consensus.pow2Phase2FirstBlockHeight=0;
            consensus.pow2Phase3FirstBlockHeight=0;
            consensus.devBlockSubsidyActivationHeight=1;
            consensus.pow2Phase4FirstBlockHeight=0;
            consensus.pow2Phase5FirstBlockHeight=0;
            consensus.deltaV3ActivationTime=1591188240;

            numGenesisWitnesses = 10;
            genesisWitnessWeightDivisor = 100;
                
            if (fIsOfficialTestnetV1)
            {                
                // Don't bother creating the genesis block if we haven't started ECC yet (e.g. we are being called from the help text)
                // We can't initialise key anyway unless the app has first initialised ECC, and the help doesn't need the genesis block, creating it twice is a waste of cpu cycles
                ECC_Start();
                {
                    CMutableTransaction txNew(CTransaction::CURRENT_VERSION);
                    txNew.vin.resize(1);
                    txNew.vin[0].prevout.SetNull();
                    txNew.vin[0].segregatedSignatureData.stack.clear();
                    txNew.vin[0].segregatedSignatureData.stack.push_back(std::vector<unsigned char>());
                    CVectorWriter(0, 0, txNew.vin[0].segregatedSignatureData.stack[0], 0) << VARINT(0);
                    txNew.vin[0].segregatedSignatureData.stack.push_back(ParseHex("4f6e206a616e756172692031737420746865204475746368206c6f73742074686572652062656c6f7665642047756c64656e"));
                    
                    {
                        std::string sKey = sTestnetParams;
                        sKey.resize(32, 0);
                        genesisWitnessPrivKey.Set((unsigned char*)&sKey[0],(unsigned char*)&sKey[0]+32, true);
                        
                        CTxOut renewedWitnessTxOutput;
                        renewedWitnessTxOutput.SetType(CTxOutType::PoW2WitnessOutput);
                        renewedWitnessTxOutput.output.witnessDetails.spendingKeyID = genesisWitnessPrivKey.GetPubKey().GetID();
                        renewedWitnessTxOutput.output.witnessDetails.witnessKeyID = genesisWitnessPrivKey.GetPubKey().GetID();
                        renewedWitnessTxOutput.output.witnessDetails.lockFromBlock = 1;
                        renewedWitnessTxOutput.output.witnessDetails.lockUntilBlock = 900000;
                        renewedWitnessTxOutput.output.witnessDetails.failCount = 0;
                        renewedWitnessTxOutput.output.witnessDetails.actionNonce = 1;
                        renewedWitnessTxOutput.nValue=0;
                        for (uint32_t i=0; i<numGenesisWitnesses;++i)
                        {
                            txNew.vout.push_back(renewedWitnessTxOutput);
                        }
                    }

                    genesis.nTime    = 1596003003;
                    genesis.nBits    = 524287999;
                    genesis.nNonce   = 4131389449;
                    genesis.nVersion = 536870912;
                    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
                    genesis.hashPrevBlock.SetNull();
                    genesis.hashMerkleRoot = BlockMerkleRoot(genesis.vtx.begin(), genesis.vtx.end());
                    genesis.hashMerkleRootPoW2Witness = BlockMerkleRoot(genesis.vtx.begin(), genesis.vtx.end());
                    genesis.witnessHeaderPoW2Sig.resize(65);
                    genesis.nTimePoW2Witness = 1596003004;
                    genesis.nVersionPoW2Witness = 536870912;
                }
                ECC_Stop();
            }
            else
            {               
                // Don't bother creating the genesis block if we haven't started ECC yet (e.g. we are being called from the help text)
                // We can't initialise key anyway unless the app has first initialised ECC, and the help doesn't need the genesis block, creating it twice is a waste of cpu cycles
                ECC_Start();
                {
                    CMutableTransaction txNew(CTransaction::CURRENT_VERSION);
                    txNew.vin.resize(1);
                    txNew.vin[0].prevout.SetNull();
                    txNew.vin[0].segregatedSignatureData.stack.clear();
                    txNew.vin[0].segregatedSignatureData.stack.push_back(std::vector<unsigned char>());
                    CVectorWriter(0, 0, txNew.vin[0].segregatedSignatureData.stack[0], 0) << VARINT(0);
                    txNew.vin[0].segregatedSignatureData.stack.push_back(ParseHex("4f6e206a616e756172692031737420746865204475746368206c6f73742074686572652062656c6f7665642047756c64656e"));
                    
                    {
                        std::string sKey = sTestnetParams;
                        sKey.resize(32, 0);
                        genesisWitnessPrivKey.Set((unsigned char*)&sKey[0],(unsigned char*)&sKey[0]+32, true);
                        
                        CTxOut renewedWitnessTxOutput;
                        renewedWitnessTxOutput.SetType(CTxOutType::PoW2WitnessOutput);
                        renewedWitnessTxOutput.output.witnessDetails.spendingKeyID = genesisWitnessPrivKey.GetPubKey().GetID();
                        renewedWitnessTxOutput.output.witnessDetails.witnessKeyID = genesisWitnessPrivKey.GetPubKey().GetID();
                        renewedWitnessTxOutput.output.witnessDetails.lockFromBlock = 1;
                        renewedWitnessTxOutput.output.witnessDetails.lockUntilBlock = 900000;
                        renewedWitnessTxOutput.output.witnessDetails.failCount = 0;
                        renewedWitnessTxOutput.output.witnessDetails.actionNonce = 1;
                        renewedWitnessTxOutput.nValue=0;
                        for (uint32_t i=0; i<numGenesisWitnesses;++i)
                        {
                            txNew.vout.push_back(renewedWitnessTxOutput);
                        }
                    }

                    genesis.nTime    = seedTimestamp;
                    genesis.nBits    = arith_uint256((~arith_uint256(0) >> 10)).GetCompact();
                    genesis.nNonce   = 0;
                    genesis.nVersion = 536870912;
                    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
                    genesis.hashPrevBlock.SetNull();
                    genesis.hashMerkleRoot = BlockMerkleRoot(genesis.vtx.begin(), genesis.vtx.end());
                    genesis.hashMerkleRootPoW2Witness = BlockMerkleRoot(genesis.vtx.begin(), genesis.vtx.end());
                    genesis.witnessHeaderPoW2Sig.resize(65);

                    uint256 foundBlockHash;
                    std::atomic<uint64_t> halfHashCounter=0;
                    std::atomic<uint64_t> nThreadCounter=0;
                    bool interrupt=false;
                    sigma_context generateContext(defaultSigmaSettings, defaultSigmaSettings.arenaSizeKb, std::max(GetNumCores(), 1));
                    generateContext.prepareArenas(genesis);
                    generateContext.mineBlock(&genesis, halfHashCounter, foundBlockHash, interrupt);
                    
                    genesis.nTimePoW2Witness = genesis.nTime+1;
                    genesis.nVersionPoW2Witness = genesis.nVersion;
                }
                ECC_Stop();
            }
            consensus.hashGenesisBlock = genesis.GetHashPoW2();
            LogPrintf("genesis nonce: %d\n",genesis.nNonce);
            LogPrintf("genesis time: %d\n",genesis.nTime);
            LogPrintf("genesis bits: %d\n",genesis.nBits);
            LogPrintf("genesis hash: %s\n", consensus.hashGenesisBlock.ToString().c_str());

            pchMessageStart[0] = targetInterval & 0xFF;
            pchMessageStart[1] = (seedTimestamp >> 8) & 0xFF;
            pchMessageStart[2] = (seedTimestamp >> 16) & 0xFF;
            pchMessageStart[3] = sTestnetParams[0];

            LogPrintf("pchMessageStart (aka magic bytes). decimal:[%d %d %d %d] hex:[%#x %#x %#x %#x]\n", pchMessageStart[0], pchMessageStart[1], pchMessageStart[2], pchMessageStart[3], pchMessageStart[0], pchMessageStart[1], pchMessageStart[2], pchMessageStart[3]);
        }

        vAlertPubKey = ParseHex("06087071e40ddf2ecbdf1ae40f536fa8f78e9383006c710dd3ecce957a3cb9292038d0840e3be5042a6b863f75dfbe1cae8755a0f7887ae459af689f66caacab52");
        nDefaultPort = 9923;
        nPruneAfterHeight = 1000;

        vFixedSeeds.clear();
        vSeeds.clear();
        //vSeeds.push_back(CDNSSeedData("seed 0", "testseed.gulden.blue"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,65);// 'T'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,127);// 't'
        base58Prefixes[POW2_WITNESS_ADDRESS] = std::vector<unsigned char>(1,63);// 'S'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,65+128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fUseSyncCheckpoints = false;


        checkpointData = {
            { 0, { genesis.GetHashPoW2(), genesis.nTime } }
        };

        if (fIsOfficialTestnetV1)
        {
            checkpointData = {
            {      0, { uint256S("0x924d4d8a9601594dc5a2adf14cefc354e9df230cb5215d5f42b61d0da60e0b03"), 1596003004 } },
            };
            consensus.defaultAssumeValid = uint256S("0x924d4d8a9601594dc5a2adf14cefc354e9df230cb5215d5f42b61d0da60e0b03");
        }

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 60;//1 minute
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].type = Consensus::DEPLOYMENT_POW;
        
        //Never activate
        defaultSigmaSettings.activationDate = std::numeric_limits<uint64_t>::max();


        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].type = Consensus::DEPLOYMENT_POW;

        consensus.fixedRewardReductionHeight=40600;
        consensus.pow2Phase2FirstBlockHeight=40800;
        consensus.pow2Phase3FirstBlockHeight=50000;
        consensus.devBlockSubsidyActivationHeight=50100;
        consensus.pow2Phase4FirstBlockHeight=50500;
        consensus.pow2Phase5FirstBlockHeight=50500;
        consensus.deltaV3ActivationTime=1591188240;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfc; // 'N' + 0xb0
        pchMessageStart[1] = 0xfe; // 'L' + 0xb0
        pchMessageStart[2] = 0xf7; // 'G' + 0xb0
        pchMessageStart[3] = 0xFF; // 0xFF
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 2, UintToArith256(consensus.powLimit).GetCompact(), 1, 0);
        consensus.hashGenesisBlock = genesis.GetHashPoW2();
        assert(consensus.hashGenesisBlock == uint256S("0x3e4b830e0f75f7b72060ae5ebcc22fdf5df57c7e2350a2669ac4f8a2d734e1bc"));
        assert(genesis.hashMerkleRoot == uint256S("0x4bed0bcb3e6097445ae68d455137625bb66f0e7ba06d9db80290bf72e3d6dcf8"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fUseSyncCheckpoints = false;

        checkpointData = {
            { 0, { genesis.GetHashPoW2(), genesis.nTime } }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,60);// 'R'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,122);// 'r'
        base58Prefixes[POW2_WITNESS_ADDRESS] = std::vector<unsigned char>(1,123);// 'r'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,60+128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void FreeParams()
{
    globalChainParams = nullptr;
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
