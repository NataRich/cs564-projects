import os
import sys
from re import sub
from json import loads

from data_models import *


columnSeparator = "|"
resultdir = os.path.join(os.getcwd(), 'result')

# Dictionary of months used for date transformation
MONTHS = {'Jan':'01','Feb':'02','Mar':'03','Apr':'04','May':'05','Jun':'06',\
        'Jul':'07','Aug':'08','Sep':'09','Oct':'10','Nov':'11','Dec':'12'}


def isJson(f):
    """
    Returns true if a file ends in .json
    """

    return len(f) > 5 and f[-5:] == '.json'


def transformMonth(mon):
    """
    Converts month to a number, e.g. 'Dec' to '12'
    """
    if mon in MONTHS:
        return MONTHS[mon]
    else:
        return mon


def transformDttm(dttm):
    """
    Transforms a timestamp from Mon-DD-YY HH:MM:SS to YYYY-MM-DD HH:MM:SS
    """
    dttm = dttm.strip().split(' ')
    dt = dttm[0].split('-')
    date = '20' + dt[2] + '-'
    date += transformMonth(dt[0]) + '-' + dt[1]
    return date + ' ' + dttm[1]


def transformDollar(money):
    """
    Transform a dollar value amount from a string like $3,453.23 to XXXXX.xx
    """

    if money == None or len(money) == 0:
        return money
    return sub(r'[^\d.]', '', money)


class Parser:
    def __init__(self):
        self.c = CategoryData()
        self.u = UserData()
        self.b = BidData()
        self.i = ItemData()
        self.ci = CategorizationData()

    def _log_if_none(self, key, value, mark):
        assert key is not None
        assert mark is not None
        return value

    def _get(self, d, k, mark, key=None):
        return self._log_if_none(key or k, d[k] if k in d else None, mark)

    def _parse_category(self, item, item_id):
        assert item is not None
        assert item_id is not None

        categories = self._get(item, 'Category', item_id)
        if categories is not None:
            for category in categories:
                cate_id = self.c.add_if_absent(category)
                self.ci.add_if_absent(item_id, cate_id)

    def _parse_bid(self, item, item_id):
        assert item is not None
        assert item_id is not None

        bids = self._get(item, 'Bids', item_id)
        if bids is None:
            return

        for e in bids:
            bid = self._get(e, 'Bid', item_id)
            bdr = self._get(bid, 'Bidder', item_id)

            user_id = self._get(bdr, 'UserID', item_id, key='BidderID')
            rating = self._get(bdr, 'Rating', item_id, key='BidderRating')
            location = self._get(bdr, 'Location', item_id, key='BidderLocation')
            country = self._get(bdr, 'Country', item_id, key='BidderCountry')
            self.u.smart_add(user_id, rating, location, country)

            time = transformDttm(self._get(bid, 'Time', item_id, key='BidTime'))
            amount = transformDollar(self._get(bid, 'Amount', item_id, key='BidAmount'))
            self.b.add(user_id, item_id, time, amount)

    def _parse_item(self, item, item_id):
        assert item is not None
        assert item_id is not None

        seller = self._get(item, 'Seller', item_id)
        
        user_id = self._get(seller, 'UserID', item_id, key='SellerID')
        rating = self._get(seller, 'Rating', item_id, key='SellerRating')
        location = self._get(item, 'Location', item_id)
        country = self._get(item, 'Country', item_id)
        self.u.smart_add(user_id, rating, location, country)

        name = self._get(item, 'Name', item_id)
        currently = transformDollar(self._get(item, 'Currently', item_id))
        buy_price = transformDollar(self._get(item, 'Buy_Price', item_id))
        first_bid = transformDollar(self._get(item, 'First_Bid', item_id))
        num_bids = self._get(item, 'Num_Bids', item_id)
        started = transformDttm(self._get(item, 'Started', item_id))
        ends = transformDttm(self._get(item, 'Ends', item_id))
        desc = self._get(item, 'Description', item_id)
        self.i.add(item_id, user_id, name, currently, buy_price, first_bid,
                num_bids, started, ends, desc)

    def parse(self, items):
        assert items is not None

        for item in items:
            item_id = self._get(item, 'ItemID', 'at start')
            assert item_id is not None

            self._parse_category(item, item_id)
            self._parse_bid(item, item_id)
            self._parse_item(item, item_id)

    def flush(self):
        self.c.flush(os.path.join(resultdir, 'category.dat'))
        self.u.flush(os.path.join(resultdir, 'user.dat'))
        self.b.flush(os.path.join(resultdir, 'bid.dat'))
        self.i.flush(os.path.join(resultdir, 'item.dat'))
        self.ci.flush(os.path.join(resultdir, 'categorization.dat'))


parser = Parser()


def parseJson(json_file):
    """
    Parses a single json file.
    """
    with open(json_file, 'r') as f:
        items = loads(f.read())['Items'] # creates a Python dictionary of Items for the supplied json file
        parser.parse(items)


def main(argv):
    if len(argv) < 2:
        print >> sys.stderr, 'Usage: python parser.py <path to json files>'
        sys.exit(1)
    # loops over all .json files in the argument
    for f in argv[1:]:
        if isJson(f):
            parseJson(f)
            print("Success parsing " + f)

    parser.flush()
    print("Success all")


if __name__ == '__main__':
    main(sys.argv)
