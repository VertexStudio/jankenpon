##################################
#
#   Play a round
#
#   $1 round
#   $2 peer
#   $3 host
#
#   Play options:
#       1 - Jan
#       2 - Ken
#       3 - Pon


ROUND=$1

echo "------------------------------------------------------------------------------"
echo "BEFORE"
cleos get table jankenpon player.rosa game

echo "------------------------------------------------------------------------------"
echo "player.rosa commit"
PLAYER_ROSA_PLAYED_ROUND=$3
PLAYER_ROSA_SECRET_ROUND=$(openssl rand 31 -hex)0$PLAYER_ROSA_PLAYED_ROUND
PLAYER_ROSA_COMMIT_ROUND=$(echo -n $PLAYER_ROSA_SECRET_ROUND | xxd -r -p | sha2 -256 -q)
echo "PLAYER_ROSA_SECRET_ROUND: $PLAYER_ROSA_SECRET_ROUND"
echo "PLAYER_ROSA_PLAYED_ROUND: $PLAYER_ROSA_PLAYED_ROUND"
echo "PLAYER_ROSA_COMMIT_ROUND: $PLAYER_ROSA_COMMIT_ROUND"
cleos push action jankenpon commit '["player.jose", "player.rosa", "'"$ROUND"'", "player.rosa", "'"$PLAYER_ROSA_COMMIT_ROUND"'"]' -p player.rosa

echo "------------------------------------------------------------------------------"
echo "player.jose commit"
PLAYER_JOSE_PLAYED_ROUND=$2
PLAYER_JOSE_SECRET_ROUND=$(openssl rand 31 -hex)0$PLAYER_JOSE_PLAYED_ROUND
PLAYER_JOSE_COMMIT_ROUND=$(echo -n $PLAYER_JOSE_SECRET_ROUND | xxd -r -p | sha2 -256 -q)
echo "PLAYER_JOSE_SECRET_ROUND: $PLAYER_JOSE_SECRET_ROUND"
echo "PLAYER_JOSE_PLAYED_ROUND: $PLAYER_JOSE_PLAYED_ROUND"
echo "PLAYER_JOSE_COMMIT_ROUND: $PLAYER_JOSE_COMMIT_ROUND"
cleos push action jankenpon commit '["player.jose", "player.rosa", "'"$ROUND"'", "player.jose", "'"$PLAYER_JOSE_COMMIT_ROUND"'"]' -p player.jose

echo "------------------------------------------------------------------------------"
echo "player.rosa reveal"
cleos push action jankenpon reveal '["player.jose", "player.rosa", "'"$ROUND"'", "player.rosa", "'"$PLAYER_ROSA_SECRET_ROUND"'", "'"$PLAYER_ROSA_PLAYED_ROUND"'"]' -p player.rosa

echo "------------------------------------------------------------------------------"
echo "player.jose reveal"
cleos push action jankenpon reveal '["player.jose", "player.rosa", "'"$ROUND"'", "player.jose", "'"$PLAYER_JOSE_SECRET_ROUND"'", "'"$PLAYER_JOSE_PLAYED_ROUND"'"]' -p player.jose

echo "------------------------------------------------------------------------------"
echo "AFTER"
cleos get table jankenpon player.rosa game