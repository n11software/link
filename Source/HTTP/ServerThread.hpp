#ifndef HTTP_ServerThread
#define HTTP_ServerThread

/*
 * Manages HTTP connections.
 */
namespace HTTP {
  class ServerThread {
    public:
      ServerThread(int port, int maxConnections);
      int getPort(), getMaxConnections();
    private:
      int port, maxConnections;
  };
}

#endif