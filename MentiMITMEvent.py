from mitmproxy import ctx
from mitmproxy.http import HTTPResponse
from mitmproxy.net.http.headers import Headers
import json
from subprocess import Popen
import re
import os

reqMethod  = 'POST'
prettyHost = 'www.menti.com'
preProtoco = 'https://'
voteSuffix = '/core/vote'
prettyURL  = preProtoco + prettyHost + voteSuffix
stopHost   = '133.7.133.7'

class MentiIntercepter:
    def __init__(self):
        self.process = None
        self.regex   = re.compile(preProtoco + prettyHost + '/(.+)$')

    def request(self, flow):
        if (flow.request.pretty_host == prettyHost and flow.request.method == reqMethod):
            if (flow.request.pretty_url == prettyURL):
                if self.process != None:
                    self.process.terminate()

                flow.request.decode()
                uForm = json.loads(flow.request.content)

                quizMatch = self.regex.match(flow.request.headers['referer'])
                quiz      = quizMatch.group(1)

                runDir  = os.path.dirname(os.path.abspath(__file__))
                runExec = os.path.join(runDir, 'MentiMemer')
                
                ctx.log.info("Calling \'" + runExec + "\'")

                self.process = Popen([runExec, quiz, uForm['question'], uForm['question_type'], json.dumps(uForm['vote'], ensure_ascii=False)], cwd=runDir)
        elif (flow.request.pretty_host == stopHost):
            if self.process != None:
                self.process.terminate()
                self.process = None

            flow.response = HTTPResponse("HTTP/1.1", 200, "OK", Headers(Content_Type="text/html"), b'<!DOCTYPE html><html><body><big style=\"font-size: 5em\">Stopped auto-send</big></body></html>')

addons = [
    MentiIntercepter()
]
