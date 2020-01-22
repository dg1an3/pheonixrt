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
