// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from libunity.djinni

#import "DBPeerRecord.h"


@implementation DBPeerRecord

- (nonnull instancetype)initWithId:(int64_t)id
                                ip:(nonnull NSString *)ip
                          hostname:(nonnull NSString *)hostname
                         addrLocal:(nonnull NSString *)addrLocal
                          addrBind:(nonnull NSString *)addrBind
                       startHeight:(int64_t)startHeight
                      syncedHeight:(int64_t)syncedHeight
                      commonHeight:(int64_t)commonHeight
                     timeConnected:(int64_t)timeConnected
                        timeOffset:(int64_t)timeOffset
                           latency:(int64_t)latency
                          lastSend:(int64_t)lastSend
                       lastReceive:(int64_t)lastReceive
                         sendBytes:(int64_t)sendBytes
                      receiveBytes:(int64_t)receiveBytes
                         userAgent:(nonnull NSString *)userAgent
                          protocol:(int64_t)protocol
                          services:(int64_t)services
                           inbound:(BOOL)inbound
                       whitelisted:(BOOL)whitelisted
                           addnode:(BOOL)addnode
                         relayTxes:(BOOL)relayTxes
                          banscore:(int64_t)banscore
{
    if (self = [super init]) {
        _id = id;
        _ip = [ip copy];
        _hostname = [hostname copy];
        _addrLocal = [addrLocal copy];
        _addrBind = [addrBind copy];
        _startHeight = startHeight;
        _syncedHeight = syncedHeight;
        _commonHeight = commonHeight;
        _timeConnected = timeConnected;
        _timeOffset = timeOffset;
        _latency = latency;
        _lastSend = lastSend;
        _lastReceive = lastReceive;
        _sendBytes = sendBytes;
        _receiveBytes = receiveBytes;
        _userAgent = [userAgent copy];
        _protocol = protocol;
        _services = services;
        _inbound = inbound;
        _whitelisted = whitelisted;
        _addnode = addnode;
        _relayTxes = relayTxes;
        _banscore = banscore;
    }
    return self;
}

+ (nonnull instancetype)peerRecordWithId:(int64_t)id
                                      ip:(nonnull NSString *)ip
                                hostname:(nonnull NSString *)hostname
                               addrLocal:(nonnull NSString *)addrLocal
                                addrBind:(nonnull NSString *)addrBind
                             startHeight:(int64_t)startHeight
                            syncedHeight:(int64_t)syncedHeight
                            commonHeight:(int64_t)commonHeight
                           timeConnected:(int64_t)timeConnected
                              timeOffset:(int64_t)timeOffset
                                 latency:(int64_t)latency
                                lastSend:(int64_t)lastSend
                             lastReceive:(int64_t)lastReceive
                               sendBytes:(int64_t)sendBytes
                            receiveBytes:(int64_t)receiveBytes
                               userAgent:(nonnull NSString *)userAgent
                                protocol:(int64_t)protocol
                                services:(int64_t)services
                                 inbound:(BOOL)inbound
                             whitelisted:(BOOL)whitelisted
                                 addnode:(BOOL)addnode
                               relayTxes:(BOOL)relayTxes
                                banscore:(int64_t)banscore
{
    return [(DBPeerRecord*)[self alloc] initWithId:id
                                                ip:ip
                                          hostname:hostname
                                         addrLocal:addrLocal
                                          addrBind:addrBind
                                       startHeight:startHeight
                                      syncedHeight:syncedHeight
                                      commonHeight:commonHeight
                                     timeConnected:timeConnected
                                        timeOffset:timeOffset
                                           latency:latency
                                          lastSend:lastSend
                                       lastReceive:lastReceive
                                         sendBytes:sendBytes
                                      receiveBytes:receiveBytes
                                         userAgent:userAgent
                                          protocol:protocol
                                          services:services
                                           inbound:inbound
                                       whitelisted:whitelisted
                                           addnode:addnode
                                         relayTxes:relayTxes
                                          banscore:banscore];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p id:%@ ip:%@ hostname:%@ addrLocal:%@ addrBind:%@ startHeight:%@ syncedHeight:%@ commonHeight:%@ timeConnected:%@ timeOffset:%@ latency:%@ lastSend:%@ lastReceive:%@ sendBytes:%@ receiveBytes:%@ userAgent:%@ protocol:%@ services:%@ inbound:%@ whitelisted:%@ addnode:%@ relayTxes:%@ banscore:%@>", self.class, (void *)self, @(self.id), self.ip, self.hostname, self.addrLocal, self.addrBind, @(self.startHeight), @(self.syncedHeight), @(self.commonHeight), @(self.timeConnected), @(self.timeOffset), @(self.latency), @(self.lastSend), @(self.lastReceive), @(self.sendBytes), @(self.receiveBytes), self.userAgent, @(self.protocol), @(self.services), @(self.inbound), @(self.whitelisted), @(self.addnode), @(self.relayTxes), @(self.banscore)];
}

@end
