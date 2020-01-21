import time
import urllib.request
from urllib.request import urlopen, build_opener, HTTPCookieProcessor
import re
from http.cookiejar import CookieJar
import datetime
import twitter

cj = CookieJar()
opener = build_opener(HTTPCookieProcessor(cj))
opener.addheaders = [('User-agent', 'Mozilla/5.0')]

def main():
    try:
        page = 'http://www.huffingtonpost.com/feeds/index.xml'
        sourceCode = opener.open(page).read()
        #print sourceCode

        try:
            titles = re.findall(r'',sourceCode)
            links = re.findall(r'(.*?)',sourceCode)
            for title in titles:
                print(title)
            for link in links:
                print(link)
        except Exception(e):
            print(str(e))

    except Exception(e):
        print(str(e))
        pass

def twitter_main():
    api = twitter.Api(consumer_key='NiZXmlG2nndUQBTiGtog6yKpl',
                        consumer_secret='OELIJzFxLb6GJxslMpqUJk1425d312qPQpteswUi468elQl6P3',
                        access_token_key='188802609-3fe7LslQiV7eg6OnNTLCdS3v3HzIyexUM158F7pz',
                        access_token_secret='doNsqnZfL2ILFAcuLodvKaUdCTKNtqyvMaqMMgFFaaGPv')
    print(api)

    tweets = api.GetUserTimeline(
        # screen_name='Reuters', 
        screen_name='TheEconomist',
        count=1000, 
        # since_id=1212059334378692609, max_id=1212085763829047297
        since_id=1200000000000000000, max_id=1201000000000000000
    )
    import pprint
    pprint.pprint([(t.text, t.id, t.created_at) for t in tweets])
    # search_result = api.GetSearch(raw_query='q=brexit%20(from%3ATheEconomist)')
    # print(search_result)

twitter_main()
# main()
		