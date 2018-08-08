#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>

const uint8_t k_tie = 0;
const uint8_t k_jan = 1;
const uint8_t k_ken = 2;
const uint8_t k_pon = 3;

uint8_t plays[3][3] = {
    {k_tie, k_ken, k_jan},
    {k_ken, k_tie, k_pon},
    {k_jan, k_pon, k_tie},
};

using namespace eosio;

class jankenpon : public eosio::contract
{
  private:
    static bool is_equal(const checksum256 &a, const checksum256 &b)
    {
        return memcmp((void *)&a, (const void *)&b, sizeof(checksum256)) == 0;
    }

    static bool is_zero(const checksum256 &a)
    {
        const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&a);
        return p64[0] == 0 && p64[1] == 0 && p64[2] == 0 && p64[3] == 0;
    }

  public:
    using contract::contract;

    jankenpon(account_name self) : contract(self), games(_self, _self) {}

    //@abi table
    struct game
    {
        account_name peer;
        account_name host;
        uint8_t peerwins;
        uint8_t hostwins;
        uint8_t round;
        checksum256 peercommit;
        checksum256 hostcommit;
        checksum256 peersecret;
        checksum256 hostsecret;
        auto primary_key() const { return peer; }
        EOSLIB_SERIALIZE(game, (peer)(host)(peerwins)(hostwins)(round)(peercommit)(hostcommit)(peersecret)(hostsecret))
    };

    typedef eosio::multi_index<N(game), game> game_index;

    game_index games;

    //@abi action
    void create(const account_name &peer, const account_name &host)
    {
        require_auth(host);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        game_index existing_host_games(_self, host);
        auto itr = existing_host_games.find(peer);
        eosio_assert(itr == existing_host_games.end(), "game already exists");
        existing_host_games.emplace(host, [&](auto &g) {
            g.peer = peer;
            g.host = host;
            g.peerwins = 0;
            g.hostwins = 0;
            g.round = 1;
            memset(&g.peercommit, 0, sizeof(checksum256));
            memset(&g.hostcommit, 0, sizeof(checksum256));
            memset(&g.peersecret, 0, sizeof(checksum256));
            memset(&g.hostsecret, 0, sizeof(checksum256));
        });
    }

    //@abi commit
    void commit(const account_name &peer, const account_name &host, uint8_t round, const account_name &player, const checksum256 &commitment)
    {
        require_auth(player);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        game_index existing_host_games(_self, host);
        auto itr = existing_host_games.find(peer);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exists");
        existing_host_games.modify(itr, host, [&](auto &g) {
            eosio_assert((g.round == round), "invalid round");
            if (player == peer)
            {
                eosio_assert(is_zero(g.peercommit), "peer already commited");
                g.peercommit = commitment;
            }
            else if (player == host)
            {
                eosio_assert(is_zero(g.hostcommit), "host already committed");
                g.hostcommit = commitment;
            }
        });
    }

    //@abi reveal
    void reveal(const account_name &peer, const account_name &host, uint8_t round, const account_name &player, const checksum256 &secret)
    {
        require_auth(player);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        game_index existing_host_games(_self, host);
        auto itr = existing_host_games.find(peer);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exists");
        uint8_t play = ((uint8_t *)&secret)[31];
        eosio_assert((play >= 1 && play <= 3), "invalid play");
        existing_host_games.modify(itr, host, [&](auto &g) {
            eosio_assert((g.round == round), "invalid round");
            if (player == peer)
            {
                eosio_assert(!is_zero(g.peercommit), "peer hasn't committed");
                eosio_assert(is_zero(g.peersecret), "peer already revealed");
                g.peersecret = secret;
                assert_sha256((char *)&secret, sizeof(secret), (const checksum256 *)&g.peercommit);
            }
            else if (player == host)
            {
                eosio_assert(!is_zero(g.hostcommit), "host hasn't committed");
                eosio_assert(is_zero(g.hostsecret), "host already revealed");
                g.hostsecret = secret;
                assert_sha256((char *)&secret, sizeof(secret), (const checksum256 *)&g.hostcommit);
            }
            if (!is_zero(g.peersecret) && !is_zero(g.hostsecret))
            {
                g.round += 1;
                uint8_t peerplay = ((uint8_t *)&g.peersecret)[31];
                uint8_t hostplay = ((uint8_t *)&g.hostsecret)[31];
                g.peerwins += peerplay == plays[peerplay - 1][hostplay - 1] ? 1 : 0;
                g.hostwins += hostplay == plays[peerplay - 1][hostplay - 1] ? 1 : 0;
                memset(&g.peercommit, 0, sizeof(checksum256));
                memset(&g.hostcommit, 0, sizeof(checksum256));
                memset(&g.peersecret, 0, sizeof(checksum256));
                memset(&g.hostsecret, 0, sizeof(checksum256));
            }
        });
    }
};

EOSIO_ABI(jankenpon, (create)(commit)(reveal))
