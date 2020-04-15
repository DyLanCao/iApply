# -*- coding: UTF-8 -*-

import urllib2
import urllib
import json
import re
import base64

class CSPotter:
    def __init__(self, platform):
        self.platform = platform

    def checksum(self, data):
        checksum = 0
        for x in data:
            checksum = (checksum + ord(x))

        return checksum

    def post(self, url, data):
        request = urllib2.urlopen(
            url = url,
            data=data)

        resp = request.read()
        return resp

    def buildPayload(self, macAddress):
        if self.platform == "BES2300":
            data = "ProjectID=13422022&LoginAccount=TonyCore&LoginPassword=TonyCore0612&ChipName=BES2300&MACAddress=" + macAddress + "&SDKName=CSpotter&"
        elif self.platform == "WT200H":
            data = "ProjectID=52805332&LoginAccount=TonyCore&LoginPassword=TonyCore0627&ChipName=WT200H&MACAddress=" + macAddress + "&SDKName=CSpotter&"
        checksum = self.checksum(data)
        print "checksum : %d" % checksum
        self.data = data+"Checksum="+str(checksum)
        print self.data


    def sendReq(self, macAddress):
        URL =  "http://176.32.76.222/cgi-bin/AuthorizationManagement_TonyCore/AuthorizationManagement.cgi"
        self.buildPayload(macAddress)
        self.result = self.post(URL, self.data)
        return self.result

    def DataGet(self):
        reStr = 'Data=(?P<data>.*)'
        match = re.search(reStr, self.result)
        if match:
            data = match.groupdict()['data']
            return base64.b64decode(data)


if __name__ == "__main__":
    cs = CSPotter("WT200H")
    with open("licence.bin", 'w') as f:
        cs.sendReq("12-34-56-78-22-22")
        f.write(cs.DataGet())
