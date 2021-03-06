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

class [[eosio::contract("jankenpon")]] Jankenpon : public eosio::contract
{
  private:
    static bool is_equal(const capi_checksum256 &a, const capi_checksum256 &b)
    {
        return memcmp((void *)&a, (const void *)&b, sizeof(capi_checksum256)) == 0;
    }

    static bool is_zero(const capi_checksum256 &a)
    {
        const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&a);
        return p64[0] == 0 && p64[1] == 0 && p64[2] == 0 && p64[3] == 0;
    }

  public:
    using contract::contract;

    Jankenpon(name self, name code, datastream<const char*> ds) 
        : contract(self, code, ds), games(self, code.value) {}

    struct [[eosio::table]] game
    {
        uint64_t peer;
        uint64_t host;
        uint8_t peerwins;
        uint8_t hostwins;
        uint8_t round;
        capi_checksum256 peercommit;
        capi_checksum256 hostcommit;
        capi_checksum256 peersecret;
        capi_checksum256 hostsecret;
        uint64_t primary_key() const { return peer; }
        EOSLIB_SERIALIZE(game, (peer)(host)(peerwins)(hostwins)(round)(peercommit)(hostcommit)(peersecret)(hostsecret))
    };

    typedef eosio::multi_index<name("games"), game> game_index;

    game_index games;

    [[eosio::action]]
    void create(const name &peer, const name &host)
    {
        require_auth(host);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        game_index existing_host_games(_self, host.value);
        auto itr = existing_host_games.find(peer.value);
        eosio_assert(itr == existing_host_games.end(), "game already exists");
        existing_host_games.emplace(host, [&](auto &g) {
            g.peer = peer.value;
            g.host = host.value;
            g.peerwins = 0;
            g.hostwins = 0;
            g.round = 1;
            memset(&g.peercommit, 0, sizeof(capi_checksum256));
            memset(&g.hostcommit, 0, sizeof(capi_checksum256));
            memset(&g.peersecret, 0, sizeof(capi_checksum256));
            memset(&g.hostsecret, 0, sizeof(capi_checksum256));
        });
    }

    [[eosio::action]]
    void commit(const name &peer, const name &host, uint8_t round, const name &player, const capi_checksum256 &commitment)
    {
        require_auth(player);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        game_index existing_host_games(_self, host.value);
        auto itr = existing_host_games.find(peer.value);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exist");
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

    [[eosio::action]]
    void reveal(const name &peer, const name &host, uint8_t round, const name &player, const capi_checksum256 &secret)
    {
        require_auth(player);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        
        game_index existing_host_games(_self, host.value);
        auto itr = existing_host_games.find(peer.value);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exist");
        
        uint8_t play = ((uint8_t *)&secret)[31];
        eosio_assert((play >= 1 && play <= 3), "invalid play");
        existing_host_games.modify(itr, host, [&](auto &g) {

            eosio_assert(!is_zero(g.hostcommit), "host hasn't committed");
            eosio_assert(!is_zero(g.peercommit), "peer hasn't committed");

            eosio_assert((g.round == round), "invalid round");
            if (player == peer)
            {
                eosio_assert(is_zero(g.peersecret), "peer already revealed");
                assert_sha256((char *)&secret, sizeof(secret), (const capi_checksum256 *)&g.peercommit);
                g.peersecret = secret;
            }
            else if (player == host)
            {
                
                eosio_assert(is_zero(g.hostsecret), "host already revealed");
                assert_sha256((char *)&secret, sizeof(secret), (const capi_checksum256 *)&g.hostcommit);
                g.hostsecret = secret;
            }
            if (!is_zero(g.peersecret) && !is_zero(g.hostsecret))
            {
                g.round += 1;
                uint8_t peerplay = ((uint8_t *)&g.peersecret)[31];
                uint8_t hostplay = ((uint8_t *)&g.hostsecret)[31];
                g.peerwins += peerplay == plays[peerplay - 1][hostplay - 1] ? 1 : 0;
                g.hostwins += hostplay == plays[peerplay - 1][hostplay - 1] ? 1 : 0;
                memset(&g.peercommit, 0, sizeof(capi_checksum256));
                memset(&g.hostcommit, 0, sizeof(capi_checksum256));
                memset(&g.peersecret, 0, sizeof(capi_checksum256));
                memset(&g.hostsecret, 0, sizeof(capi_checksum256));
            }
        });
    }

    [[eosio::action]] 
    void endgame(const name &peer, const name &host) 
    {
        require_auth(host);
        eosio_assert(peer != host, "peer shouldn't be the same as host");
        
        game_index existing_host_games(_self, host.value);
        auto itr = existing_host_games.find(peer.value);

        eosio_assert(itr != existing_host_games.end(), "game doesn't exist");

        eosio_assert(is_zero((*itr).hostcommit), "host has committed. Round isn't over yet! Reveal is required!");
        eosio_assert(is_zero((*itr).peercommit), "peer has committed. Round isn't over yet! Reveal is required!");

        existing_host_games.erase(itr);
    }
};

EOSIO_DISPATCH(Jankenpon, (create)(commit)(reveal)(endgame))