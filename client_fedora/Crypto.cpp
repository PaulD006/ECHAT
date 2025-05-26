#include "Crypto.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdexcept>

namespace {
    QString sessionPrivPem;
}

void Crypto::init() {
    OpenSSL_add_all_algorithms();
}

QByteArray Crypto::generateRandom(int length) {
    QByteArray buf(length, 0);
    if(!RAND_bytes(reinterpret_cast<unsigned char*>(buf.data()), length))
        throw std::runtime_error("RAND_bytes failed");
    return buf;
}

QByteArray Crypto::generateSessionKeyPair(QString &publicPem, QString &privatePem) {
    // RSA 2048
    RSA *rsa = RSA_new();
    BIGNUM *e = BN_new();
    BN_set_word(e, RSA_F4);
    RSA_generate_key_ex(rsa, 2048, e, nullptr);
    BN_free(e);

    BIO *pubBio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(pubBio, rsa);
    char *pubData; long pubLen = BIO_get_mem_data(pubBio, &pubData);
    publicPem = QString::fromUtf8(pubData, pubLen);
    BIO_free(pubBio);

    BIO *privBio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(privBio, rsa, nullptr, nullptr, 0, nullptr, nullptr);
    char *privData; long privLen = BIO_get_mem_data(privBio, &privData);
    privatePem = QString::fromUtf8(privData, privLen);
    BIO_free(privBio);

    QByteArray result; // not used directly
    sessionPrivPem = privatePem;
    RSA_free(rsa);
    return result;
}

QString Crypto::getSessionPrivateKey() {
    return sessionPrivPem;
}

QByteArray Crypto::encryptRSA(const QString &pubPem, const QByteArray &data) {
    BIO *bio = BIO_new_mem_buf(pubPem.toUtf8().data(), pubPem.toUtf8().size());
    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    int keyLen = RSA_size(rsa);
    QByteArray out(keyLen, 0);
    int len = RSA_public_encrypt(data.size(), 
                                 (unsigned char*)data.constData(),
                                 (unsigned char*)out.data(),
                                 rsa, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);
    if(len<0) throw std::runtime_error("RSA encrypt failed");
    out.truncate(len);
    return out;
}

QByteArray Crypto::decryptRSA(const QString &privPem, const QByteArray &data) {
    BIO *bio = BIO_new_mem_buf(privPem.toUtf8().data(), privPem.toUtf8().size());
    RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    int keyLen = RSA_size(rsa);
    QByteArray out(keyLen, 0);
    int len = RSA_private_decrypt(data.size(),
                                  (unsigned char*)data.constData(),
                                  (unsigned char*)out.data(),
                                  rsa, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);
    if(len<0) throw std::runtime_error("RSA decrypt failed");
    out.truncate(len);
    return out;
}

QByteArray Crypto::aesEncrypt(const QByteArray &key, const QByteArray &iv, const QByteArray &plain) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int outLen1=0, outLen2=0;
    QByteArray out(plain.size()+EVP_MAX_BLOCK_LENGTH, 0);
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr,
                       (unsigned char*)key.constData(),
                       (unsigned char*)iv.constData());
    EVP_EncryptUpdate(ctx, (unsigned char*)out.data(), &outLen1,
                      (unsigned char*)plain.constData(), plain.size());
    EVP_EncryptFinal_ex(ctx, (unsigned char*)(out.data()+outLen1), &outLen2);
    EVP_CIPHER_CTX_free(ctx);
    out.truncate(outLen1+outLen2);
    return out;
}

QByteArray Crypto::aesDecrypt(const QByteArray &key, const QByteArray &iv, const QByteArray &cipher) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int outLen1=0, outLen2=0;
    QByteArray out(cipher.size(), 0);
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr,
                       (unsigned char*)key.constData(),
                       (unsigned char*)iv.constData());
    EVP_DecryptUpdate(ctx, (unsigned char*)out.data(), &outLen1,
                      (unsigned char*)cipher.constData(), cipher.size());
    EVP_DecryptFinal_ex(ctx, (unsigned char*)(out.data()+outLen1), &outLen2);
    EVP_CIPHER_CTX_free(ctx);
    out.truncate(outLen1+outLen2);
    return out;
}