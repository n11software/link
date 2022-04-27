#ifndef HTTP

/*
 * Manages HTTP connections.
 */
namespace HTTP {
  class ServerThread {
    public:
      ServerThread(int port, int maxConnections, int maxQueuedConnections, bool* Running);
      int getPort(), getMaxConnections(), getMaxQueuedConnections();
      bool isRunning();
    private:
      int port, maxConnections, maxQueuedConnections;
      bool* Running;
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