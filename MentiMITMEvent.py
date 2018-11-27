from mitmproxy import ctx
import json
from subprocess import Popen
import re
import os

reqMethod  = 'POST'
prettyHost = 'www.menti.com'
preProtoco = 'https://'
voteSuffix = '/core/vote'
prettyURL  = preProtoco + prettyHost + voteSuffix

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

                self.process = Popen([runExec, quiz, uForm['question'], uForm['question_type'], uForm['vote']], cwd=runDir)

addons = [
    MentiIntercepter()
]
