cleos wallet unlock -n debug --password PW5JGqrHwsAphXLGcZ4rHAJqQxmGQXdL42puB5Vjo14odGspwYFvo

# setup params
EOS_BUILD_PATH=$HOME/eos/build
EOS_KEY=EOS8k4wDWyrq9yPUL17YKqThf5F5VVMs5c8diRZQfGKG9698dd6na

# initial chain contracts
echo "\nBooting Chain Contracts"
cleos set contract eosio ${EOS_BUILD_PATH}/contracts/eosio.bios -p eosio

# dealer account
echo "\nCreating dealer account"
cleos create account eosio dealer ${EOS_KEY} ${EOS_KEY}
cleos set contract dealer jankenpon -p dealer

# Create players accounts
echo "\nCreating players accounts"
cleos create account eosio player.jose ${EOS_KEY} ${EOS_KEY}
cleos create account eosio player.rosa ${EOS_KEY} ${EOS_KEY}

cleos push action dealer create '["player.jose", "player.rosa"]' -p player.rosa

# ./play.sh 1 1 2
# ./play.sh 2 2 1
# ./play.sh 3 2 3
# ./play.sh 4 3 3
# ./play.sh 5 1 1
# ./play.sh 6 3 1
