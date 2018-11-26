from mitmproxy import ctx

class MentiIntercepter:
    def __init__(self):
        self.process = None

    def request(self, flow):
        if (flow.request.host == "https://www.menti.com/core/vote" and flow.request.method == "POST"):
            ctx.log.info("Form: " + flow.request.urlencoded_form)

addons = [
    MentiIntercepter()
]
