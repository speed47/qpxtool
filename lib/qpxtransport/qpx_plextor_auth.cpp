#include <cstring>

/* Always select the vendored Openwall implementation, even if a parent build defines this Openwall compatibility
 * switch globally. */
#ifdef HAVE_OPENSSL
#undef HAVE_OPENSSL
#endif

extern "C" {
#include "third_party/openbsd/blf.h"
#include "third_party/openwall/md5.h"
}

#include "qpx_plextor_auth.h"

namespace {

constexpr unsigned char PLEXTOR_AUTH_SEED[16] = {
    0x3d, 0x14, 0x4a, 0x9c, 0x2b, 0x4a, 0x53, 0x63, 0x25, 0x98, 0xf2, 0xa5, 0xc4, 0xad, 0x03, 0xcf,
};

void md5_16(const unsigned char input[16], unsigned char digest[16]) {
	MD5_CTX context;
	MD5_Init(&context);
	MD5_Update(&context, input, 16);
	MD5_Final(digest, &context);
}

} // namespace

/* PlexTools Professional XL 3.16 computes the protected-command response as:
 *   key      = MD5(seed)
 *   clear    = Blowfish-ECB-DECRYPT(challenge[0..7], key) ||
 *              Blowfish-ECB-DECRYPT(challenge[8..15], key)
 *   response = MD5(clear)
 * The recovered transform matches all 43 captured Premium2 D4/D5 exchanges. */
int qpx_plextor_auth_response(const unsigned char challenge[16], unsigned char response[16]) {
	unsigned char key[16];
	unsigned char cleartext[16];
	blf_ctx context;

	md5_16(PLEXTOR_AUTH_SEED, key);
	std::memcpy(cleartext, challenge, sizeof(cleartext));
	blf_key(&context, key, sizeof(key));
	blf_ecb_decrypt(&context, cleartext, sizeof(cleartext));
	md5_16(cleartext, response);

	return 0;
}
