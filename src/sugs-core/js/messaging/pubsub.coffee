###
Copyright 2011 Jeffery Olson <olson.jeffery@gmail.com>. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of
    conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice, this list
    of conditions and the following disclaimer in the documentation and/or other materials
    provided with the distribution.

THIS SOFTWARE IS PROVIDED BY JEFFERY OLSON <OLSON.JEFFERY@GMAIL.COM> ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
JEFFERY OLSON <OLSON.JEFFERY@GMAIL.COM> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Jeffery Olson <olson.jeffery@gmail.com>.
###
global = this
msgExMsgHandlers = {}
worker = require 'worker'

global.__processIncomingMessage = (msgId, jsonData) ->
  if typeof msgExMsgHandlers[msgId] == "undefined"
    throw "No handlers for msg ex registered handler of #{msgId}"
  else
    data = JSON.parse jsonData
    _.each msgExMsgHandlers[msgId], (cb) ->
      cb.call data, data.msg, data.meta

class PubSubMessenger
  _publishBroadcastNew: (workerIds, endpoint, data, isUdp) ->
    dataJson = JSON.stringify data
    global.native_publishBroadcast workerIds, endpoint, dataJson, isUdp
  _publishSingle: (targetWorkerId, msgId, data, isUdp) ->
    dataJson = JSON.stringify data
    global.native_publishSingle targetWorkerId, msgId, dataJson, isUdp

  subscribe : (msgId, callback) ->
    if typeof msgExMsgHandlers[msgId] == "undefined"
      msgExMsgHandlers[msgId] = []
    msgExMsgHandlers[msgId].push callback
    global.native_subscribe msgId

  publish: (targetWorkerId, msgId, msg, isUdp) ->
    isUdpVal = false
    if (typeof isUdp) != 'undefined'
      isUdpVal = isUdp
    if typeof msg == "undefined" #called with only two args.. msgId = msg payload
      throw "msg is undefined. provider targetWorkerId: '#{targetWorkerId}'. please specify the recipient(s), msgId and msg"
    else
      data =
        msg: msg
        meta:
          sender: worker.current.getId()
          msgId: msgId
      if targetWorkerId instanceof Array
        if targetWorkerId.length == 0
          throw "You must provide at least one worker Id to publish a message to"
        @_publishBroadcastNew targetWorkerId, msgId, data, isUdpVal
      else
        @_publishSingle targetWorkerId, msgId, data, isUdpVal

  publishFast: (targetWorkerId, msgId, msg) ->
    @publish targetWorkerId, msgId, msg, true

retModule = new PubSubMessenger()
retModule.testImpl = new PubSubMessenger()
retModule.testImpl._publishSingle = ->
retModule.testImpl._publishBroadcastOld = ->
retModule.testImpl._publishBroadcastNew = ->

return retModule