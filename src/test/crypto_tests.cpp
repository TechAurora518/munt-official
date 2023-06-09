// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "crypto/chacha20.h"
#include "crypto/ripemd160.h"
#include "crypto/sha1.h"
#include "crypto/sha256.h"
#include "crypto/sha512.h"
#include "crypto/hmac_sha256.h"
#include "crypto/hmac_sha512.h"
#include "crypto/scrypt/crypto_scrypt.h"
#include "random.h"
#include "util/strencodings.h"
#include "test/test.h"

#include <cryptopp/config.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

#include <vector>

#include <boost/test/unit_test.hpp>
#include <openssl/evp.h>

BOOST_FIXTURE_TEST_SUITE(crypto_tests, BasicTestingSetup)

template<typename Hasher, typename In, typename Out>
void TestVector(const Hasher &h, const In &in, const Out &out) {
    Out hash;
    BOOST_CHECK(out.size() == h.OUTPUT_SIZE);
    hash.resize(out.size());
    {
        // Test that writing the whole input string at once works.
        Hasher(h).Write((const uint8_t*)in.data(), in.size()).Finalize(hash.data());
        BOOST_CHECK(hash == out);
    }
    for (int i=0; i<32; i++) {
        // Test that writing the string broken up in random pieces works.
        Hasher hasher(h);
        size_t pos = 0;
        while (pos < in.size()) {
            size_t len = InsecureRandRange((in.size() - pos + 1) / 2 + 1);
            hasher.Write((const uint8_t*)in.data() + pos, len);
            pos += len;
            if (pos > 0 && pos + 2 * out.size() > in.size() && pos < in.size()) {
                // Test that writing the rest at once to a copy of a hasher works.
                Hasher(hasher).Write((const uint8_t*)in.data() + pos, in.size() - pos).Finalize(hash.data());
                BOOST_CHECK(hash == out);
            }
        }
        hasher.Finalize(hash.data());
        BOOST_CHECK(hash == out);
    }
}

static void TestSHA1(const std::string &in, const std::string &hexout) { TestVector(CSHA1(), in, ParseHex(hexout));}
static void TestSHA256(const std::string &in, const std::string &hexout) { TestVector(CSHA256(), in, ParseHex(hexout));}
static void TestSHA512(const std::string &in, const std::string &hexout) { TestVector(CSHA512(), in, ParseHex(hexout));}
static void TestRIPEMD160(const std::string &in, const std::string &hexout) { TestVector(CRIPEMD160(), in, ParseHex(hexout));}

static void TestHMACSHA256(const std::string &hexkey, const std::string &hexin, const std::string &hexout) {
    std::vector<unsigned char> key = ParseHex(hexkey);
    TestVector(CHMAC_SHA256(key.data(), key.size()), ParseHex(hexin), ParseHex(hexout));
}

static void TestHMACSHA512(const std::string &hexkey, const std::string &hexin, const std::string &hexout) {
    std::vector<unsigned char> key = ParseHex(hexkey);
    TestVector(CHMAC_SHA512(key.data(), key.size()), ParseHex(hexin), ParseHex(hexout));
}

static void TestAES128(const std::string &hexkey, const std::string &hexin, const std::string &hexout)
{
    std::vector<unsigned char> key = ParseHex(hexkey);
    std::vector<unsigned char> in = ParseHex(hexin);
    std::vector<unsigned char> correctout = ParseHex(hexout);
    std::vector<unsigned char> buf, buf2;

    assert(key.size() == 16);
    assert(in.size() == 16);
    assert(correctout.size() == 16);
    
    CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKey(&key[0], key.size());
    buf.resize(correctout.size());
    buf2.resize(correctout.size());
    enc.ProcessData(&buf[0], &in[0], correctout.size());
    BOOST_CHECK_EQUAL(HexStr(buf), HexStr(correctout));
    
    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKey(&key[0], key.size());
    dec.ProcessData(&buf2[0], &buf[0], correctout.size());
    BOOST_CHECK_EQUAL(HexStr(buf2), HexStr(in));
}

static void TestAES256(const std::string &hexkey, const std::string &hexin, const std::string &hexout)
{
    std::vector<unsigned char> key = ParseHex(hexkey);
    std::vector<unsigned char> in = ParseHex(hexin);
    std::vector<unsigned char> correctout = ParseHex(hexout);
    std::vector<unsigned char> buf;

    assert(key.size() == 32);
    assert(in.size() == 16);
    assert(correctout.size() == 16);
    
    CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKey(&key[0], key.size());
    buf.resize(correctout.size());
    enc.ProcessData(&buf[0], &in[0], correctout.size());
    BOOST_CHECK(buf == correctout);
    
    CryptoPP::ECB_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKey(&key[0], key.size());
    dec.ProcessData(&buf[0], &buf[0], correctout.size());
    BOOST_CHECK(buf == in);
}

static void TestAES128CBC(const std::string &hexkey, const std::string &hexiv, bool pad, const std::string &hexin, const std::string &hexout)
{
    std::vector<unsigned char> key = ParseHex(hexkey);
    std::vector<unsigned char> iv = ParseHex(hexiv);
    std::vector<unsigned char> in = ParseHex(hexin);
    std::vector<unsigned char> correctout = ParseHex(hexout);
    std::vector<unsigned char> realout;

    // Encrypt the plaintext and verify that it equals the cipher
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKeyWithIV(&key[0], key.size(), &iv[0]);
    if (pad)
    {
        CryptoPP::VectorSource s(in, true, new CryptoPP::StreamTransformationFilter(enc, new CryptoPP::VectorSink(realout)));
    }
    else
    {
        realout.resize(in.size());
        enc.ProcessData(&realout[0], &in[0], in.size());
    }
    BOOST_CHECK(realout.size() == correctout.size());
    BOOST_CHECK_MESSAGE(realout == correctout, HexStr(realout) + std::string(" != ") + hexout);

    // Decrypt the cipher and verify that it equals the plaintext
    std::vector<unsigned char> decrypted;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKeyWithIV(&key[0], key.size(), &iv[0]);
    if (pad)
    {
        CryptoPP::VectorSource s(correctout, true, new CryptoPP::StreamTransformationFilter(dec, new CryptoPP::VectorSink(decrypted)));
    }
    else
    {
        decrypted.resize(correctout.size());
        dec.ProcessData(&decrypted[0], &correctout[0], correctout.size());
    }
    BOOST_CHECK(decrypted.size() == in.size());
    BOOST_CHECK_MESSAGE(decrypted == in, HexStr(decrypted) + std::string(" != ") + hexin);

    // Encrypt and re-decrypt substrings of the plaintext and verify that they equal each-other
    for(std::vector<unsigned char>::iterator i(in.begin()); i != in.end(); ++i)
    {
        std::vector<unsigned char> sub(i, in.end());
        std::vector<unsigned char> subout;
        CryptoPP::VectorSource encryptSource(sub, true, new CryptoPP::StreamTransformationFilter(enc, new CryptoPP::VectorSink(subout)));
        
        std::vector<unsigned char> subdecrypted;
        CryptoPP::VectorSource decryptSource(subout, true, new CryptoPP::StreamTransformationFilter(dec, new CryptoPP::VectorSink(subdecrypted)));

        BOOST_CHECK(decrypted.size() == in.size());
        BOOST_CHECK_MESSAGE(subdecrypted == sub, HexStr(subdecrypted) + std::string(" != ") + HexStr(sub));
    }
}

static void TestAES256CBC(const std::string &hexkey, const std::string &hexiv, bool pad, const std::string &hexin, const std::string &hexout)
{
    std::vector<unsigned char> key = ParseHex(hexkey);
    std::vector<unsigned char> iv = ParseHex(hexiv);
    std::vector<unsigned char> in = ParseHex(hexin);
    std::vector<unsigned char> correctout = ParseHex(hexout);
    std::vector<unsigned char> realout;

    // Encrypt the plaintext and verify that it equals the cipher
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
    enc.SetKeyWithIV(&key[0], key.size(), &iv[0]);

    if (pad)
    {
        CryptoPP::VectorSource s(in, true, new CryptoPP::StreamTransformationFilter(enc, new CryptoPP::VectorSink(realout)));
    }
    else
    {
        realout.resize(in.size());
        enc.ProcessData(&realout[0], &in[0], in.size());
    }    
    BOOST_CHECK(realout.size() == correctout.size());
    BOOST_CHECK_MESSAGE(realout == correctout, HexStr(realout) + std::string(" != ") + hexout);

    // Decrypt the cipher and verify that it equals the plaintext
    std::vector<unsigned char> decrypted;
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
    dec.SetKeyWithIV(&key[0], key.size(), &iv[0]);
    if (pad)
    {
        CryptoPP::VectorSource s(correctout, true, new CryptoPP::StreamTransformationFilter(dec, new CryptoPP::VectorSink(decrypted)));
    }
    else
    {
        decrypted.resize(correctout.size());
        dec.ProcessData(&decrypted[0], &correctout[0], correctout.size());
    }
    BOOST_CHECK(decrypted.size() == in.size());
    BOOST_CHECK_MESSAGE(decrypted == in, HexStr(decrypted) + std::string(" != ") + hexin);

    // Encrypt and re-decrypt substrings of the plaintext and verify that they equal each-other
    for(std::vector<unsigned char>::iterator i(in.begin()); i != in.end(); ++i)
    {
        std::vector<unsigned char> sub(i, in.end());
        std::vector<unsigned char> subout;
        CryptoPP::VectorSource encryptSource(sub, true, new CryptoPP::StreamTransformationFilter(enc, new CryptoPP::VectorSink(subout)));
        
        std::vector<unsigned char> subdecrypted;
        CryptoPP::VectorSource decryptSource(subout, true, new CryptoPP::StreamTransformationFilter(dec, new CryptoPP::VectorSink(subdecrypted)));
        BOOST_CHECK(decrypted.size() == in.size());
        BOOST_CHECK_MESSAGE(subdecrypted == sub, HexStr(subdecrypted) + std::string(" != ") + HexStr(sub));
    }
}

static void TestChaCha20(const std::string &hex_message, const std::string &hexkey, uint64_t nonce, uint64_t seek, const std::string& hexout)
{
    std::vector<unsigned char> key = ParseHex(hexkey);
    std::vector<unsigned char> m = ParseHex(hex_message);
    ChaCha20 rng(key.data(), key.size());
    rng.SetIV(nonce);
    rng.Seek(seek);
    std::vector<unsigned char> out = ParseHex(hexout);
    std::vector<unsigned char> outres;
    outres.resize(out.size());
    assert(hex_message.empty() || m.size() == out.size());

    // perform the ChaCha20 round(s), if message is provided it will output the encrypted ciphertext otherwise the keystream
    if (!hex_message.empty()) {
        rng.Crypt(m.data(), outres.data(), outres.size());
    } else {
        rng.Keystream(outres.data(), outres.size());
    }
    BOOST_CHECK(out == outres);
    if (!hex_message.empty()) {
        // Manually XOR with the keystream and compare the output
        rng.SetIV(nonce);
        rng.Seek(seek);
        std::vector<unsigned char> only_keystream(outres.size());
        rng.Keystream(only_keystream.data(), only_keystream.size());
        for (size_t i = 0; i != m.size(); i++) {
            outres[i] = m[i] ^ only_keystream[i];
        }
        BOOST_CHECK(out == outres);
    }
}

static std::string LongTestString()
{
    std::string ret;
    for (int i = 0; i < 200000; i++) {
        ret += (char)(i);
        ret += (char)(i >> 4);
        ret += (char)(i >> 8);
        ret += (char)(i >> 12);
        ret += (char)(i >> 16);
    }
    return ret;
}

const std::string test1 = LongTestString();

BOOST_AUTO_TEST_CASE(ripemd160_testvectors) {
    TestRIPEMD160("", "9c1185a5c5e9fc54612808977ee8f548b2258d31");
    TestRIPEMD160("abc", "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc");
    TestRIPEMD160("message digest", "5d0689ef49d2fae572b881b123a85ffa21595f36");
    TestRIPEMD160("secure hash algorithm", "20397528223b6a5f4cbc2808aba0464e645544f9");
    TestRIPEMD160("RIPEMD160 is considered to be safe", "a7d78608c7af8a8e728778e81576870734122b66");
    TestRIPEMD160("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
                  "12a053384a9c0c88e405a06c27dcf49ada62eb2b");
    TestRIPEMD160("For this sample, this 63-byte string will be used as input data",
                  "de90dbfee14b63fb5abf27c2ad4a82aaa5f27a11");
    TestRIPEMD160("This is exactly 64 bytes long, not counting the terminating byte",
                  "eda31d51d3a623b81e19eb02e24ff65d27d67b37");
    TestRIPEMD160(std::string(1000000, 'a'), "52783243c1697bdbe16d37f97f68f08325dc1528");
    TestRIPEMD160(test1, "464243587bd146ea835cdf57bdae582f25ec45f1");
}

BOOST_AUTO_TEST_CASE(sha1_testvectors) {
    TestSHA1("", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    TestSHA1("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
    TestSHA1("message digest", "c12252ceda8be8994d5fa0290a47231c1d16aae3");
    TestSHA1("secure hash algorithm", "d4d6d2f0ebe317513bbd8d967d89bac5819c2f60");
    TestSHA1("SHA1 is considered to be safe", "f2b6650569ad3a8720348dd6ea6c497dee3a842a");
    TestSHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
             "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
    TestSHA1("For this sample, this 63-byte string will be used as input data",
             "4f0ea5cd0585a23d028abdc1a6684e5a8094dc49");
    TestSHA1("This is exactly 64 bytes long, not counting the terminating byte",
             "fb679f23e7d1ce053313e66e127ab1b444397057");
    TestSHA1(std::string(1000000, 'a'), "34aa973cd4c4daa4f61eeb2bdbad27316534016f");
    TestSHA1(test1, "b7755760681cbfd971451668f32af5774f4656b5");
}

BOOST_AUTO_TEST_CASE(sha256_testvectors) {
    TestSHA256("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    TestSHA256("abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    TestSHA256("message digest",
               "f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650");
    TestSHA256("secure hash algorithm",
               "f30ceb2bb2829e79e4ca9753d35a8ecc00262d164cc077080295381cbd643f0d");
    TestSHA256("SHA256 is considered to be safe",
               "6819d915c73f4d1e77e4e1b52d1fa0f9cf9beaead3939f15874bd988e2a23630");
    TestSHA256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
               "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    TestSHA256("For this sample, this 63-byte string will be used as input data",
               "f08a78cbbaee082b052ae0708f32fa1e50c5c421aa772ba5dbb406a2ea6be342");
    TestSHA256("This is exactly 64 bytes long, not counting the terminating byte",
               "ab64eff7e88e2e46165e29f2bce41826bd4c7b3552f6b382a9e7d3af47c245f8");
    TestSHA256("As Bitcoin relies on 80 byte header hashes, we want to have an example for that.",
               "7406e8de7d6e4fffc573daef05aefb8806e7790f55eab5576f31349743cca743");
    TestSHA256(std::string(1000000, 'a'),
               "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    TestSHA256(test1, "a316d55510b49662420f49d145d42fb83f31ef8dc016aa4e32df049991a91e26");
}

BOOST_AUTO_TEST_CASE(sha512_testvectors) {
    TestSHA512("",
               "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce"
               "47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
    TestSHA512("abc",
               "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
               "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
    TestSHA512("message digest",
               "107dbf389d9e9f71a3a95f6c055b9251bc5268c2be16d6c13492ea45b0199f33"
               "09e16455ab1e96118e8a905d5597b72038ddb372a89826046de66687bb420e7c");
    TestSHA512("secure hash algorithm",
               "7746d91f3de30c68cec0dd693120a7e8b04d8073cb699bdce1a3f64127bca7a3"
               "d5db502e814bb63c063a7a5043b2df87c61133395f4ad1edca7fcf4b30c3236e");
    TestSHA512("SHA512 is considered to be safe",
               "099e6468d889e1c79092a89ae925a9499b5408e01b66cb5b0a3bd0dfa51a9964"
               "6b4a3901caab1318189f74cd8cf2e941829012f2449df52067d3dd5b978456c2");
    TestSHA512("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
               "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c335"
               "96fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445");
    TestSHA512("For this sample, this 63-byte string will be used as input data",
               "b3de4afbc516d2478fe9b518d063bda6c8dd65fc38402dd81d1eb7364e72fb6e"
               "6663cf6d2771c8f5a6da09601712fb3d2a36c6ffea3e28b0818b05b0a8660766");
    TestSHA512("This is exactly 64 bytes long, not counting the terminating byte",
               "70aefeaa0e7ac4f8fe17532d7185a289bee3b428d950c14fa8b713ca09814a38"
               "7d245870e007a80ad97c369d193e41701aa07f3221d15f0e65a1ff970cedf030");
    TestSHA512("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno"
               "ijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
               "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
               "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909");
    TestSHA512(std::string(1000000, 'a'),
               "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973eb"
               "de0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b");
    TestSHA512(test1,
               "40cac46c147e6131c5193dd5f34e9d8bb4951395f27b08c558c65ff4ba2de594"
               "37de8c3ef5459d76a52cedc02dc499a3c9ed9dedbfb3281afd9653b8a112fafc");
}

BOOST_AUTO_TEST_CASE(hmac_sha256_testvectors) {
    // test cases 1, 2, 3, 4, 6 and 7 of RFC 4231
    TestHMACSHA256("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
                   "4869205468657265",
                   "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
    TestHMACSHA256("4a656665",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
    TestHMACSHA256("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                   "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
                   "dddddddddddddddddddddddddddddddddddd",
                   "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe");
    TestHMACSHA256("0102030405060708090a0b0c0d0e0f10111213141516171819",
                   "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd"
                   "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd",
                   "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b");
    TestHMACSHA256("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaa",
                   "54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a"
                   "65204b6579202d2048617368204b6579204669727374",
                   "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54");
    TestHMACSHA256("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaa",
                   "5468697320697320612074657374207573696e672061206c6172676572207468"
                   "616e20626c6f636b2d73697a65206b657920616e642061206c61726765722074"
                   "68616e20626c6f636b2d73697a6520646174612e20546865206b6579206e6565"
                   "647320746f20626520686173686564206265666f7265206265696e6720757365"
                   "642062792074686520484d414320616c676f726974686d2e",
                   "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2");
    // Test case with key length 63 bytes.
    TestHMACSHA256("4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a6566",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "9de4b546756c83516720a4ad7fe7bdbeac4298c6fdd82b15f895a6d10b0769a6");
    // Test case with key length 64 bytes.
    TestHMACSHA256("4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "528c609a4c9254c274585334946b7c2661bad8f1fc406b20f6892478d19163dd");
    // Test case with key length 65 bytes.
    TestHMACSHA256("4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "d06af337f359a2330deffb8e3cbe4b5b7aa8ca1f208528cdbd245d5dc63c4483");
}

BOOST_AUTO_TEST_CASE(hmac_sha512_testvectors) {
    // test cases 1, 2, 3, 4, 6 and 7 of RFC 4231
    TestHMACSHA512("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
                   "4869205468657265",
                   "87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cde"
                   "daa833b7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854");
    TestHMACSHA512("4a656665",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea250554"
                   "9758bf75c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737");
    TestHMACSHA512("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                   "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
                   "dddddddddddddddddddddddddddddddddddd",
                   "fa73b0089d56a284efb0f0756c890be9b1b5dbdd8ee81a3655f83e33b2279d39"
                   "bf3e848279a722c806b485a47e67c807b946a337bee8942674278859e13292fb");
    TestHMACSHA512("0102030405060708090a0b0c0d0e0f10111213141516171819",
                   "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd"
                   "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd",
                   "b0ba465637458c6990e5a8c5f61d4af7e576d97ff94b872de76f8050361ee3db"
                   "a91ca5c11aa25eb4d679275cc5788063a5f19741120c4f2de2adebeb10a298dd");
    TestHMACSHA512("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaa",
                   "54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a"
                   "65204b6579202d2048617368204b6579204669727374",
                   "80b24263c7c1a3ebb71493c1dd7be8b49b46d1f41b4aeec1121b013783f8f352"
                   "6b56d037e05f2598bd0fd2215d6a1e5295e64f73f63f0aec8b915a985d786598");
    TestHMACSHA512("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                   "aaaaaa",
                   "5468697320697320612074657374207573696e672061206c6172676572207468"
                   "616e20626c6f636b2d73697a65206b657920616e642061206c61726765722074"
                   "68616e20626c6f636b2d73697a6520646174612e20546865206b6579206e6565"
                   "647320746f20626520686173686564206265666f7265206265696e6720757365"
                   "642062792074686520484d414320616c676f726974686d2e",
                   "e37b6a775dc87dbaa4dfa9f96e5e3ffddebd71f8867289865df5a32d20cdc944"
                   "b6022cac3c4982b10d5eeb55c3e4de15134676fb6de0446065c97440fa8c6a58");
    // Test case with key length 127 bytes.
    TestHMACSHA512("4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a6566",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "267424dfb8eeb999f3e5ec39a4fe9fd14c923e6187e0897063e5c9e02b2e624a"
                   "c04413e762977df71a9fb5d562b37f89dfdfb930fce2ed1fa783bbc2a203d80e");
    // Test case with key length 128 bytes.
    TestHMACSHA512("4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "43aaac07bb1dd97c82c04df921f83b16a68d76815cd1a30d3455ad43a3d80484"
                   "2bb35462be42cc2e4b5902de4d204c1c66d93b47d1383e3e13a3788687d61258");
    // Test case with key length 129 bytes.
    TestHMACSHA512("4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a6566654a6566654a6566654a6566654a6566654a6566654a6566654a656665"
                   "4a",
                   "7768617420646f2079612077616e7420666f72206e6f7468696e673f",
                   "0b273325191cfc1b4b71d5075c8fcad67696309d292b1dad2cd23983a35feb8e"
                   "fb29795e79f2ef27f68cb1e16d76178c307a67beaad9456fac5fdffeadb16e2c");
}

BOOST_AUTO_TEST_CASE(aes_testvectors) {
    // AES test vectors from FIPS 197.
    TestAES256("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", "00112233445566778899aabbccddeeff", "8ea2b7ca516745bfeafc49904b496089");

    // AES-ECB test vectors from NIST sp800-38a.
    TestAES256("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", "6bc1bee22e409f96e93d7e117393172a", "f3eed1bdb5d2a03c064b5a7e3db181f8");
    TestAES256("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", "ae2d8a571e03ac9c9eb76fac45af8e51", "591ccb10d410ed26dc5ba74a31362870");
    TestAES256("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", "30c81c46a35ce411e5fbc1191a0a52ef", "b6ed21b99ca6f4f9f153e7b1beafed1d");
    TestAES256("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", "f69f2445df4f9b17ad2b417be66c3710", "23304b7a39f9f3ff067d8d8f9e24ecc7");
}

BOOST_AUTO_TEST_CASE(aes_cbc_testvectors) {
    // NIST AES CBC 256-bit encryption test-vectors
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "000102030405060708090A0B0C0D0E0F", false, "6bc1bee22e409f96e93d7e117393172a", \
                  "f58c4c04d6e5f1ba779eabfb5f7bfbd6");
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "F58C4C04D6E5F1BA779EABFB5F7BFBD6", false, "ae2d8a571e03ac9c9eb76fac45af8e51", \
                  "9cfc4e967edb808d679f777bc6702c7d");
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "9CFC4E967EDB808D679F777BC6702C7D", false, "30c81c46a35ce411e5fbc1191a0a52ef",
                  "39f23369a9d9bacfa530e26304231461");
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "39F23369A9D9BACFA530E26304231461", false, "f69f2445df4f9b17ad2b417be66c3710", \
                  "b2eb05e2c39be9fcda6c19078c6a9d1b");

    // The same vectors with padding enabled
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "000102030405060708090A0B0C0D0E0F", true, "6bc1bee22e409f96e93d7e117393172a", \
                  "f58c4c04d6e5f1ba779eabfb5f7bfbd6485a5c81519cf378fa36d42b8547edc0");
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "F58C4C04D6E5F1BA779EABFB5F7BFBD6", true, "ae2d8a571e03ac9c9eb76fac45af8e51", \
                  "9cfc4e967edb808d679f777bc6702c7d3a3aa5e0213db1a9901f9036cf5102d2");
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "9CFC4E967EDB808D679F777BC6702C7D", true, "30c81c46a35ce411e5fbc1191a0a52ef",
                  "39f23369a9d9bacfa530e263042314612f8da707643c90a6f732b3de1d3f5cee");
    TestAES256CBC("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", \
                  "39F23369A9D9BACFA530E26304231461", true, "f69f2445df4f9b17ad2b417be66c3710", \
                  "b2eb05e2c39be9fcda6c19078c6a9d1b3f461796d6b0d6b2e0c2a72b4d80e644");
}


BOOST_AUTO_TEST_CASE(chacha20_testvector)
{
    // Test vector from RFC 7539

    // test encryption
    TestChaCha20("4c616469657320616e642047656e746c656d656e206f662074686520636c617373206f66202739393a204966204920636f756"
                 "c64206f6666657220796f75206f6e6c79206f6e652074697020666f7220746865206675747572652c2073756e73637265656e"
                 "20776f756c642062652069742e",
                 "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", 0x4a000000UL, 1,
                 "6e2e359a2568f98041ba0728dd0d6981e97e7aec1d4360c20a27afccfd9fae0bf91b65c5524733ab8f593dabcd62b3571639d"
                 "624e65152ab8f530c359f0861d807ca0dbf500d6a6156a38e088a22b65e52bc514d16ccf806818ce91ab77937365af90bbf74"
                 "a35be6b40b8eedf2785e42874d"
                 );

    // test keystream output
    TestChaCha20("", "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", 0x4a000000UL, 1,
                 "224f51f3401bd9e12fde276fb8631ded8c131f823d2c06e27e4fcaec9ef3cf788a3b0aa372600a92b57974cded2b9334794cb"
                 "a40c63e34cdea212c4cf07d41b769a6749f3f630f4122cafe28ec4dc47e26d4346d70b98c73f3e9c53ac40c5945398b6eda1a"
                 "832c89c167eacd901d7e2bf363");

    // Test vectors from https://tools.ietf.org/html/draft-agl-tls-chacha20poly1305-04#section-7
    TestChaCha20("", "0000000000000000000000000000000000000000000000000000000000000000", 0, 0,
                 "76b8e0ada0f13d90405d6ae55386bd28bdd219b8a08ded1aa836efcc8b770dc7da41597c5157488d7724e03fb8d84a376a43b"
                 "8f41518a11cc387b669b2ee6586");
    TestChaCha20("", "0000000000000000000000000000000000000000000000000000000000000001", 0, 0,
                 "4540f05a9f1fb296d7736e7b208e3c96eb4fe1834688d2604f450952ed432d41bbe2a0b6ea7566d2a5d1e7e20d42af2c53d79"
                 "2b1c43fea817e9ad275ae546963");
    TestChaCha20("", "0000000000000000000000000000000000000000000000000000000000000000", 0x0100000000000000ULL, 0,
                 "de9cba7bf3d69ef5e786dc63973f653a0b49e015adbff7134fcb7df137821031e85a050278a7084527214f73efc7fa5b52770"
                 "62eb7a0433e445f41e3");
    TestChaCha20("", "0000000000000000000000000000000000000000000000000000000000000000", 1, 0,
                 "ef3fdfd6c61578fbf5cf35bd3dd33b8009631634d21e42ac33960bd138e50d32111e4caf237ee53ca8ad6426194a88545ddc4"
                 "97a0b466e7d6bbdb0041b2f586b");
    TestChaCha20("", "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", 0x0706050403020100ULL, 0,
                 "f798a189f195e66982105ffb640bb7757f579da31602fc93ec01ac56f85ac3c134a4547b733b46413042c9440049176905d3b"
                 "e59ea1c53f15916155c2be8241a38008b9a26bc35941e2444177c8ade6689de95264986d95889fb60e84629c9bd9a5acb1cc1"
                 "18be563eb9b3a4a472f82e09a7e778492b562ef7130e88dfe031c79db9d4f7c7a899151b9a475032b63fc385245fe054e3dd5"
                 "a97a5f576fe064025d3ce042c566ab2c507b138db853e3d6959660996546cc9c4a6eafdc777c040d70eaf46f76dad3979e5c5"
                 "360c3317166a1c894c94a371876a94df7628fe4eaaf2ccb27d5aaae0ad7ad0f9d4b6ad3b54098746d4524d38407a6deb3ab78"
                 "fab78c9");
}

static void TestScrypt(const char* passwd, const char* salt, uint64_t N, uint32_t r, uint32_t p, uint32_t dkLen, std::vector<uint8_t> expectedResult)
{
    std::vector<uint8_t> buf;
    buf.resize(dkLen);
    BOOST_CHECK (crypto_scrypt((const uint8_t*)passwd, strlen(passwd), (const uint8_t*)salt, strlen(salt), N, r, p, (uint8_t*)&buf[0], dkLen) == 0);
    BOOST_CHECK (memcmp(&expectedResult[0], &buf[0], dkLen) == 0);
}

static void TestPBKDF2_SHA256(const char* passwd, const char* salt, uint64_t c, uint32_t dkLen, std::vector<uint8_t> expectedResult)
{
    std::vector<uint8_t> buf;
    buf.resize(dkLen);
    PBKDF2_SHA256((uint8_t*)passwd, strlen(passwd), (uint8_t*)salt, strlen(salt), c, (uint8_t *)&buf[0], dkLen);
    BOOST_CHECK (memcmp(&expectedResult[0], &buf[0], dkLen) == 0);
}

BOOST_AUTO_TEST_CASE(scrypt_testvector)
{
    // Test cases from tarsnap (original scrypt authors repo)
    // Superset of RFC 7914 test vectors
    TestScrypt( "", "", 16, 1, 1, 64, std::vector<uint8_t>{0x77, 0xd6, 0x57, 0x62, 0x38, 0x65, 0x7b, 0x20, 0x3b, 0x19, 0xca, 0x42, 0xc1, 0x8a, 0x04, 0x97, 0xf1, 0x6b, 0x48, 0x44, 0xe3, 0x07, 0x4a, 0xe8, 0xdf, 0xdf, 0xfa, 0x3f, 0xed, 0xe2, 0x14, 0x42, 0xfc, 0xd0, 0x06, 0x9d, 0xed, 0x09, 0x48, 0xf8, 0x32, 0x6a, 0x75, 0x3a, 0x0f, 0xc8, 0x1f, 0x17, 0xe8, 0xd3, 0xe0, 0xfb, 0x2e, 0x0d, 0x36, 0x28, 0xcf, 0x35, 0xe2, 0x0c, 0x38, 0xd1, 0x89, 0x06} );
    TestScrypt( "password", "NaCl", 1024, 8, 16, 64, std::vector<uint8_t>{0xfd, 0xba, 0xbe, 0x1c, 0x9d, 0x34, 0x72, 0x00, 0x78, 0x56, 0xe7, 0x19, 0x0d, 0x01, 0xe9, 0xfe, 0x7c, 0x6a, 0xd7, 0xcb, 0xc8, 0x23, 0x78, 0x30, 0xe7, 0x73, 0x76, 0x63, 0x4b, 0x37, 0x31, 0x62, 0x2e, 0xaf, 0x30, 0xd9, 0x2e, 0x22, 0xa3, 0x88, 0x6f, 0xf1, 0x09, 0x27, 0x9d, 0x98, 0x30, 0xda, 0xc7, 0x27, 0xaf, 0xb9, 0x4a, 0x83, 0xee, 0x6d, 0x83, 0x60, 0xcb, 0xdf, 0xa2, 0xcc, 0x06, 0x40} );
    TestScrypt( "pleaseletmein", "SodiumChloride", 16384, 8, 1, 64, std::vector<uint8_t>{0x70, 0x23, 0xbd, 0xcb, 0x3a, 0xfd, 0x73, 0x48, 0x46, 0x1c, 0x06, 0xcd, 0x81, 0xfd, 0x38, 0xeb, 0xfd, 0xa8, 0xfb, 0xba, 0x90, 0x4f, 0x8e, 0x3e, 0xa9, 0xb5, 0x43, 0xf6, 0x54, 0x5d, 0xa1, 0xf2, 0xd5, 0x43, 0x29, 0x55, 0x61, 0x3f, 0x0f, 0xcf, 0x62, 0xd4, 0x97, 0x05, 0x24, 0x2a, 0x9a, 0xf9, 0xe6, 0x1e, 0x85, 0xdc, 0x0d, 0x65, 0x1e, 0x40, 0xdf, 0xcf, 0x01, 0x7b, 0x45, 0x57, 0x58, 0x87} );
    TestScrypt( "pleaseletmein", "SodiumChloride", 1048576, 8, 1, 64, std::vector<uint8_t>{0x21, 0x01, 0xcb, 0x9b, 0x6a, 0x51, 0x1a, 0xae, 0xad, 0xdb, 0xbe, 0x09, 0xcf, 0x70, 0xf8, 0x81, 0xec, 0x56, 0x8d, 0x57, 0x4a, 0x2f, 0xfd, 0x4d, 0xab, 0xe5, 0xee, 0x98, 0x20, 0xad, 0xaa, 0x47, 0x8e, 0x56, 0xfd, 0x8f, 0x4b, 0xa5, 0xd0, 0x9f, 0xfa, 0x1c, 0x6d, 0x92, 0x7c, 0x40, 0xf4, 0xc3, 0x37, 0x30, 0x40, 0x49, 0xe8, 0xa9, 0x52, 0xfb, 0xcb, 0xf4, 0x5c, 0x6f, 0xa7, 0x7a, 0x41, 0xa4} );
    TestScrypt( "pleaseletmein", "SodiumChloride", 16, 8, 1, 64, std::vector<uint8_t>{0x25, 0xa9, 0xfa, 0x20, 0x7f, 0x87, 0xca, 0x09, 0xa4, 0xef, 0x8b, 0x9f, 0x77, 0x7a, 0xca, 0x16, 0xbe, 0xb7, 0x84, 0xae, 0x18, 0x30, 0xbf, 0xbf, 0xd3, 0x83, 0x25, 0xaa, 0xbb, 0x93, 0x77, 0xdf, 0x1b, 0xa7, 0x84, 0xd7, 0x46, 0xea, 0x27, 0x3b, 0xf5, 0x16, 0xa4, 0x6f, 0xbf, 0xac, 0xf5, 0x11, 0xc5, 0xbe, 0xba, 0x4c, 0x4a, 0xb3, 0xac, 0xc7, 0xfa, 0x6f, 0x46, 0x0b, 0x6c, 0x0f, 0x47, 0x7b} );

    // Additional scrypt test vectors from https://github.com/ricmoo/scrypt-js
    TestScrypt("password", "ricmoo", 262144, 8, 1, 32, {0xe2, 0x86, 0xed, 0x02, 0x98, 0x80, 0x8c, 0x0b, 0x4b, 0xb4, 0x27, 0x2c, 0xe9, 0x47, 0x09, 0x1b, 0x0d, 0xa0, 0x6b, 0xb5, 0x30, 0xc4, 0xcb, 0xab, 0x39, 0x23, 0xe4, 0x4f, 0xf4, 0x8b, 0xbc, 0x25} );
    TestScrypt("very-log-password-over-64-bytes-0123456789012345678901234567890123456789012345678901234567890123", "SodiumChloride", 1048576, 8, 1, 64, {0x4b, 0x6e, 0xab, 0x51, 0x64, 0xab, 0xdf, 0xc2, 0x96, 0x6a, 0xd0, 0x95, 0x4a, 0xe9, 0x35, 0x2b, 0xac, 0x57, 0xcd, 0x95, 0x3b, 0x79, 0x1e, 0xef, 0xf4, 0x55, 0xd3, 0xee, 0xd9, 0x58, 0x02, 0xa3, 0x1f, 0xd4, 0x98, 0xb5, 0x2d, 0xa4, 0x30, 0xe6, 0x1e, 0xd2, 0xaa, 0xab, 0xd5, 0xbc, 0x8b, 0x1a, 0x8e, 0xee, 0xe6, 0x6e, 0xd1, 0x1c, 0x2b, 0xd6, 0x09, 0x87, 0x7b, 0xe4, 0x02, 0x10, 0xe9, 0xa0} );
    TestScrypt("pleaseletmein", "very-long-password-over-64-bytes-0123456789012345678901234567890123456789012345678901234567890123", 1048576, 8, 1, 64, {0xec, 0x1d, 0x40, 0x3e, 0x82, 0xc0, 0x12, 0x54, 0xb9, 0xc5, 0xba, 0x84, 0xac, 0x06, 0x95, 0x8b, 0x32, 0x3a, 0xc9, 0xf5, 0x26, 0x65, 0x02, 0x4f, 0x1e, 0xa6, 0xed, 0x1e, 0xdf, 0x7a, 0xa6, 0x39, 0xe6, 0x98, 0x48, 0x1e, 0x4c, 0xe4, 0xbf, 0x59, 0xf7, 0xab, 0xc3, 0xeb, 0x8c, 0x01, 0xde, 0x0b, 0xa0, 0x94, 0xfe, 0x24, 0x90, 0xe3, 0xfa, 0xe6, 0xd2, 0x9f, 0x5c, 0x9e, 0x5f, 0x69, 0x78, 0x68} );
    TestScrypt("very-log-password-over-64-bytes-0123456789012345678901234567890123456789012345678901234567890123", "very-long-password-over-64-bytes-0123456789012345678901234567890123456789012345678901234567890123", 1048576, 8, 1, 64, {0xbd, 0x27, 0x85, 0xea, 0xf7, 0x4e, 0x4e, 0x18, 0x83, 0xa3, 0xdd, 0x92, 0x33, 0x63, 0x46, 0xd4, 0x80, 0x20, 0x0b, 0x7b, 0x6d, 0x0b, 0x79, 0x04, 0x36, 0x9c, 0x60, 0x75, 0x57, 0xaa, 0x2b, 0x14, 0xb4, 0xcd, 0xc9, 0x0f, 0xc8, 0xb8, 0xf0, 0xd4, 0x90, 0x62, 0x03, 0xa5, 0xbd, 0x6e, 0x06, 0x4a, 0xdd, 0x86, 0xaa, 0xc9, 0xac, 0x2f, 0xc4, 0x77, 0x7a, 0x5a, 0x68, 0x3a, 0x0e, 0xd4, 0x0e, 0xf9} );

    // RFC 7914 test vectors
    TestPBKDF2_SHA256("passwd", "salt", 1, 64, std::vector<uint8_t>{0x55, 0xac, 0x04, 0x6e, 0x56, 0xe3, 0x08, 0x9f, 0xec, 0x16, 0x91, 0xc2, 0x25, 0x44, 0xb6, 0x05, 0xf9, 0x41, 0x85, 0x21, 0x6d, 0xde, 0x04, 0x65, 0xe6, 0x8b, 0x9d, 0x57, 0xc2, 0x0d, 0xac, 0xbc, 0x49, 0xca, 0x9c, 0xcc, 0xf1, 0x79, 0xb6, 0x45, 0x99, 0x16, 0x64, 0xb3, 0x9d, 0x77, 0xef, 0x31, 0x7c, 0x71, 0xb8, 0x45, 0xb1, 0xe3, 0x0b, 0xd5, 0x09, 0x11, 0x20, 0x41, 0xd3, 0xa1, 0x97, 0x83} );
    TestPBKDF2_SHA256("Password", "NaCl", 80000, 64, {0x4d, 0xdc, 0xd8, 0xf6, 0x0b, 0x98, 0xbe, 0x21, 0x83, 0x0c, 0xee, 0x5e, 0xf2, 0x27, 0x01, 0xf9, 0x64, 0x1a, 0x44, 0x18, 0xd0, 0x4c, 0x04, 0x14, 0xae, 0xff, 0x08, 0x87, 0x6b, 0x34, 0xab, 0x56, 0xa1, 0xd4, 0x25, 0xa1, 0x22, 0x58, 0x33, 0x54, 0x9a, 0xdb, 0x84, 0x1b, 0x51, 0xc9, 0xb3, 0x17, 0x6a, 0x27, 0x2b, 0xde, 0xbb, 0xa1, 0xd0, 0x78, 0x47, 0x8f, 0x62, 0xb3, 0x97, 0xf3, 0x3c, 0x8d} );
}

BOOST_AUTO_TEST_CASE(countbits_tests)
{
    FastRandomContext ctx;
    for (unsigned int i = 0; i <= 64; ++i) {
        if (i == 0) {
            // Check handling of zero.
            BOOST_CHECK_EQUAL(CountBits(0), 0U);
        } else if (i < 10) {
            for (uint64_t j = (uint64_t)1 << (i - 1); (j >> i) == 0; ++j) {
                // Exhaustively test up to 10 bits
                BOOST_CHECK_EQUAL(CountBits(j), i);
            }
        } else {
            for (int k = 0; k < 1000; k++) {
                // Randomly test 1000 samples of each length above 10 bits.
                uint64_t j = ((uint64_t)1) << (i - 1) | ctx.randbits(i - 1);
                BOOST_CHECK_EQUAL(CountBits(j), i);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
