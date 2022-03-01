#ifndef Headers
#include <iostream>
#include <string.h>
#include <vector>

/*
 * HTTP Headers.
 */
class Headers {
  public:
    Headers(int ClientSocket);
    void Send(std::string Message);
  private:
    int ClientSocket;
    std::string FinalHeaders, Body, AcceptCharset, AcceptEncoding, AcceptLanguage, AcceptPatch, AcceptPost, 
    AcceptRanges, Accept, AccessControlAllowHeaders, AccessControlAllowMethods, AccessControlAllowOrigin, 
    AccessControlExposeHeaders, AccessControlRequestMethod, Allow, AltSvc, Authorization, CacheControl,
    Connection, ContentDisposition, ContentLocation, ContentRange, ContentSecurityPolicyReportOnly, 
    ContentSecurityPolicy, ContentType, Cookie, Cookie2, CrossOriginEmbedderPolicy, CrossOriginOpenerPolicy, 
    CrossOriginResourcePolicy, Date, Digest, ECT, ETag, Expect, Expires, FeaturePolicy, Forwarded, From, Host,
    Link, Location, NEL, Origin, Pragma, ProxyAuthenticate, ProxyAuthorization, PublicKeyPinsReportOnly, 
    PublicKeyPins, Range, Referer, ReferrerPolicy, RetryAfter, SecCHUAArch, SecCHUABitness, SecCHUAFullVersion, 
    SecCHUAModel, SecCHUAPlatformVersion, SecCHUAPlatform, SecFetchDest, SecFetchMode, SecFetchSite,
    SecWebSocketAccept, ServerTiming, Server, SetCookie, SetCookie2, SourceMap, StrictTransportSecurity, 
    TimingAllowOrigin, Trailer, UserAgent, WantDigest, Warning, XContentTypeOptions, XForwardedProto, 
    XFrameOptions, XXSSProtection;
    int AcceptCHLifetime, AccessControlMaxAge, Age, ContentDPR, ContentLength, DNT, EarlyData, LargeAllocation,
    RTT, ViewportWidth, Width;
    float DeviceMemory, Downlink, DPR;
    std::vector<std::string> AcceptCH, AccessControlRequestHeaders, ClearSiteData, ContentEncoding, 
    ContentLanguage, IfMatch, IfModifiedSince, IfNoneMatch, IfRange, IfUnmodifiedSince, KeepAlive,
    LastModified, SecCHUAFullVersionList, SecCHUA, TE, TransferEncoding, Upgrade, Vary, Via, WWWAuthenticate,
    XForwardedFor, XForwardedHost;
    std::string ExpectCT[3];
    bool AccessControlAllowCredentials, SaveData, SecCHUAMobile, SecFetchUser, UpgradeInsecureRequests,
    XDNSPrefetchControl;
    char Tk;
};

#endif