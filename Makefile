zhwkre/zhwkre_bubble_sort.o: zhwkre/algorithm/bubble_sort.c
	@echo "   gcc   zhwkre/zhwkre_bubble_sort.o"
	@gcc -c  -o zhwkre/zhwkre_bubble_sort.o zhwkre/algorithm/bubble_sort.c
zhwkre/zhwkre_bss.o: zhwkre/bss/bss.c
	@echo "   gcc   zhwkre/zhwkre_bss.o"
	@gcc -c  -o zhwkre/zhwkre_bss.o zhwkre/bss/bss.c
zhwkre/zhwkre_mutex.o: zhwkre/concurrent/mutex.c
	@echo "   gcc   zhwkre/zhwkre_mutex.o"
	@gcc -c  -o zhwkre/zhwkre_mutex.o zhwkre/concurrent/mutex.c
zhwkre/zhwkre_threading.o: zhwkre/concurrent/threading.c
	@echo "   gcc   zhwkre/zhwkre_threading.o"
	@gcc -c  -o zhwkre/zhwkre_threading.o zhwkre/concurrent/threading.c
zhwkre/zhwkre_list.o: zhwkre/containers/list.c
	@echo "   gcc   zhwkre/zhwkre_list.o"
	@gcc -c  -o zhwkre/zhwkre_list.o zhwkre/containers/list.c
zhwkre/zhwkre_unordered_map.o: zhwkre/containers/unordered_map.c
	@echo "   gcc   zhwkre/zhwkre_unordered_map.o"
	@gcc -c  -o zhwkre/zhwkre_unordered_map.o zhwkre/containers/unordered_map.c
zhwkre/zhwkre_socket.o: zhwkre/network/socket.c
	@echo "   gcc   zhwkre/zhwkre_socket.o"
	@gcc -c  -o zhwkre/zhwkre_socket.o zhwkre/network/socket.c
zhwkre/zhwkre_tcp.o: zhwkre/network/tcp.c
	@echo "   gcc   zhwkre/zhwkre_tcp.o"
	@gcc -c  -o zhwkre/zhwkre_tcp.o zhwkre/network/tcp.c
zhwkre/zhwkre_udp.o: zhwkre/network/udp.c
	@echo "   gcc   zhwkre/zhwkre_udp.o"
	@gcc -c  -o zhwkre/zhwkre_udp.o zhwkre/network/udp.c
zhwkre/zhwkre_serialization.o: zhwkre/serialization/serialization.c
	@echo "   gcc   zhwkre/zhwkre_serialization.o"
	@gcc -c  -o zhwkre/zhwkre_serialization.o zhwkre/serialization/serialization.c
zhwkre/zhwkre_utils.o: zhwkre/utils/utils.c
	@echo "   gcc   zhwkre/zhwkre_utils.o"
	@gcc -c  -o zhwkre/zhwkre_utils.o zhwkre/utils/utils.c
prot.o: prot.c
	@echo "   gcc   prot.o"
	@gcc -c -g -o prot.o prot.c
server.o: server.c
	@echo "   gcc   server.o"
	@gcc -c -g -o server.o server.c
client.o: client.c
	@echo "   gcc   client.o"
	@gcc -c -g -o client.o client.c
mutextest.o: mutextest.c
	@echo "   gcc   mutextest.o"
	@gcc -c -g -o mutextest.o mutextest.c
client: client.o prot.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_udp.o zhwkre/zhwkre_list.o zhwkre/zhwkre_utils.o 
	@echo "   LD   client"
	@gcc -pthread -o client client.o prot.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_udp.o zhwkre/zhwkre_list.o zhwkre/zhwkre_utils.o 
server: server.o prot.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_udp.o zhwkre/zhwkre_list.o zhwkre/zhwkre_utils.o 
	@echo "   LD   server"
	@gcc -pthread -o server server.o prot.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_udp.o zhwkre/zhwkre_list.o zhwkre/zhwkre_utils.o 
mutest: mutextest.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o 
	@echo "   LD   mutest"
	@gcc -pthread -o mutest mutextest.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o 
clean:
	@echo "  CLEAN   all files."
	-@rm zhwkre/zhwkre_bubble_sort.o zhwkre/zhwkre_bss.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_list.o zhwkre/zhwkre_unordered_map.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_tcp.o zhwkre/zhwkre_udp.o zhwkre/zhwkre_serialization.o zhwkre/zhwkre_utils.o prot.o server.o client.o mutextest.o client server mutest  || true
