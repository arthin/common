LWIP=/home/arthin/work/arm/lwip-1.4.1
mkdir -p lwip/include/lwip
cp $LWIP/src/include/lwip/* lwip/include/lwip
mkdir -p lwip/include/ipv4/lwip
cp $LWIP/src/include/ipv4/lwip/* lwip/include/ipv4/lwip
mkdir -p lwip/include/netif
cp $LWIP/src/include/netif/* lwip/include/netif
mkdir -p lwip/api
cp $LWIP/src/api/* lwip/api
mkdir -p lwip/core
cp $LWIP/src/core/* lwip/core
mkdir -p lwip/core/ipv4
cp $LWIP/src/core/ipv4/* lwip/core/ipv4
mkdir -p lwip/core/netif
cp $LWIP/src/netif/etharp.c lwip/core/netif
