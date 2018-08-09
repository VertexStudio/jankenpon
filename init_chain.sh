cleos wallet unlock -n debug --password PW5JGqrHwsAphXLGcZ4rHAJqQxmGQXdL42puB5Vjo14odGspwYFvo

# setup params
EOS_KEY=EOS8k4wDWyrq9yPUL17YKqThf5F5VVMs5c8diRZQfGKG9698dd6na

# jankenpon account
echo "\nCreating jankenpon account"
cleos create account eosio jankenpon ${EOS_KEY} ${EOS_KEY}
cleos set contract jankenpon ../jankenpon -p jankenpon

# Create players accounts
echo "\nCreating players accounts"
cleos create account eosio player.jose ${EOS_KEY} ${EOS_KEY}
cleos create account eosio player.rosa ${EOS_KEY} ${EOS_KEY}

cleos push action jankenpon create '["player.jose", "player.rosa"]' -p player.rosa

# ./play.sh 1 1 2
# ./play.sh 2 2 1
# ./play.sh 3 2 3
# ./play.sh 4 3 3
# ./play.sh 5 1 1
# ./play.sh 6 3 1
