#ifndef HTTP

/*
 * Manages HTTP connections.
 */
namespace HTTP {
  class ServerThread {
    public:
      ServerThread(int port, int maxConnections, int maxQueuedConnections);
      int getPort(), getMaxConnections(), getMaxQueuedConnections();
    private:
      int port, maxConnections, maxQueuedConnections;
  };

  class ClientData {
    public:
      ClientData(ServerThread* serverThread, int socket);
      ServerThread* getServerThread();
      int getSocket();
    private:
      ServerThread* serverThread;
      int socket;
  };
}

#endif