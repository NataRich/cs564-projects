import os
from re import sub


safernull = '"NULL"'
columnSeparator = "|"
reportdir = os.path.join(os.getcwd(), 'report')
resultdir = os.path.join(os.getcwd(), 'result')

# Dictionary of months used for date transformation
MONTHS = {'Jan':'01','Feb':'02','Mar':'03','Apr':'04','May':'05','Jun':'06',\
        'Jul':'07','Aug':'08','Sep':'09','Oct':'10','Nov':'11','Dec':'12'}


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


def saferstr(string):
    """This is a util function that makes a normal string safer.

    This function will first replace every double quotation mark in the
    original string with two double quotation marks, i.e., escaping the double
    quotation mark. Then, it will surround the resultant string with another
    pair of double quotation marks.

    Example:
        'st"r'  --->  '"st""r"'

    Args:
        string (str): The original string to be processed.

    Returns:
        The original string with double quotes surrounded and original double
        quotes escaped.
    """

    if string is None:
        return safernull
    return '"' + string.replace('"', '""') + '"'


class CategoryData:
    _counter = 0
    _ln = '{}|{}'

    def __init__(self):
        self.uids     = []
        self.names    = []

    def __str__(self):
        return str(self.lineify())

    def next_uid(self):
        uid = CategoryData._counter
        CategoryData._counter += 1
        return uid

    def add_if_absent(self, name):
        assert name is not None

        safername = saferstr(name)
        for i in range(len(self.names)):
            if self.names[i] == safername:
                return self.uids[i]

        uid = self.next_uid()
        self.uids.append(uid)
        self.names.append(safername)
        
        return uid

    def lineify(self):
        return [CategoryData._ln.format(uid, name) 
                for uid, name in zip(self.uids, self.names)]

    def flush(self, path):
        with open(path, 'w+') as f:
            for i in range(len(self.uids)):
                ln = CategoryData._ln.format(self.uids[i], self.names[i])
                f.write(f'{ln}\n')

        self.uids  = []
        self.names = []


class UserData:
    _ln = '{}|{}|{}|{}'

    def __init__(self):
        self.uids      = []
        self.ratings   = []
        self.locations = []
        self.countries = []

    def __str__(self):
        return str(self.lineify())

    def smart_add(self, uid, rating, location, country):
        assert uid is not None

        saferid = saferstr(uid)
        for i in range(len(self.uids)):
            if self.uids[i] == saferid:
                if self.ratings[i] == safernull and rating is not None:
                    self.ratings[i] = rating
                if self.locations[i] == safernull and location is not None:
                    self.locations[i] = saferstr(location)
                if self.countries[i] == safernull and country is not None:
                    self.countries[i] = saferstr(country)
                return

        rat = rating if rating is not None else safernull

        self.uids.append(saferid)
        self.ratings.append(rat)
        self.locations.append(saferstr(location))
        self.countries.append(saferstr(country))

    def lineify(self):
        return [UserData._ln.format(uid, rating, location, country)
                for uid, rating, location, country
                in zip(self.uids, self.ratings, self.locations, self.countries)]

    def flush(self, path):
        with open(path, 'w+') as f:
            for i in range(len(self.uids)):
                ln = UserData._ln.format(self.uids[i], self.ratings[i],
                        self.locations[i], self.countries[i])
                f.write(f'{ln}\n')

        self.uids      = []
        self.ratings   = []
        self.locations = []
        self.countries = []


class BidData:
    _counter = 0
    _ln = '{}|{}|{}|{}|{}'

    def __init__(self):
        self.uids     = []
        self.user_ids = []
        self.item_ids = []
        self.times    = []
        self.amounts  = []

    def __str__(self):
        return str(self.lineify())

    def next_uid(self):
        uid = BidData._counter
        BidData._counter += 1
        return uid

    def add(self, user_id, item_id, time, amount):
        assert user_id is not None
        assert item_id is not None

        uid = self.next_uid()
        self.uids.append(uid)
        self.user_ids.append(saferstr(user_id))
        self.item_ids.append(item_id)
        self.times.append(saferstr(time))
        self.amounts.append(saferstr(amount))

        return uid

    def lineify(self):
        return [BidData._ln.format(uid, user_id, item_id, time, amount)
                for uid, user_id, item_id, time, amount
                in zip(self.uids, self.user_ids, self.item_ids, self.times,
                    self.amounts)]

    def flush(self, path):
        with open(path, 'w+') as f:
            for i in range(len(self.uids)):
                ln = BidData._ln.format(self.uids[i], self.user_ids[i],
                        self.item_ids[i], self.times[i], self.amounts[i])
                f.write(f'{ln}\n')

        self.uids     = []
        self.user_ids = []
        self.item_ids = []
        self.times    = []
        self.amounts  = []


class CategorizationData:
    _counter = 0
    _ln = '{}|{}|{}'

    def __init__(self):
        self.uids     = []
        self.item_ids = []
        self.cate_ids = []

    def __str__(self):
        return str(self.lineify())

    def next_uid(self):
        uid = CategorizationData._counter
        CategorizationData._counter += 1
        return uid

    def add_if_absent(self, item_id, cate_id):
        assert item_id is not None
        assert cate_id is not None

        for i in range(len(self.item_ids)):
            if self.item_ids[i] == item_id and self.cate_ids[i] == cate_id:
                return self.uids[i]

        uid = self.next_uid()
        self.uids.append(uid)
        self.item_ids.append(item_id)
        self.cate_ids.append(cate_id)

        return uid

    def lineify(self):
        return [CategorizationData._ln.format(uid, item_id, cate_id)
                for uid, item_id, cate_id 
                in zip(self.uids, self.item_ids, self.cate_ids)]

    def flush(self, path):
        with open(path, 'w+') as f:
            for i in range(len(self.uids)):
                ln = CategorizationData._ln.format(self.uids[i],
                        self.item_ids[i], self.cate_ids[i])
                f.write(f'{ln}\n')

        self.uids     = []
        self.item_ids = []
        self.cate_ids = []


class ItemData:
    _ln = '{}|{}|{}|{}|{}|{}|{}|{}|{}|{}'

    def __init__(self):
        self.uids       = []
        self.seller_ids = []
        self.names      = []
        self.currentlys = []
        self.buy_prices = []
        self.first_bids = []
        self.num_bidss  = []
        self.starteds   = []
        self.endss      = []
        self.descs      = []

    def __str__(self):
        return str(self.lineify())

    def add(self, uid, seller_id, name, currently, buy_price, first_bid,
            num_bids, started, ends, desc):
        assert uid is not None
        assert uid not in self.uids
        assert seller_id is not None

        self.uids.append(uid)
        self.seller_ids.append(saferstr(seller_id))
        self.names.append(saferstr(name))
        self.currentlys.append(saferstr(currently))
        self.buy_prices.append(saferstr(buy_price))
        self.first_bids.append(saferstr(first_bid))
        self.num_bidss.append(saferstr(num_bids))
        self.starteds.append(saferstr(started))
        self.endss.append(saferstr(ends))
        self.descs.append(saferstr(desc))
    
    def lineify(self):
        return [ItemData._ln.format(uid, seller_id, name, currently, buy_price,
                                    first_bid, num_bids, started, ends, desc)
                for uid, seller_id, name, currently, buy_price, first_bid,
                    num_bids, started, ends, desc
                in zip(self.uids, self.seller_ids, self.names, self.currentlys,
                    self.buy_prices, self.first_bids, self.num_bidss,
                    self.starteds, self.endss, self.descs)]

    def flush(self, path):
        with open(path, 'w+') as f:
            for i in range(len(self.uids)):
                ln = ItemData._ln.format(self.uids[i], self.seller_ids[i],
                        self.names[i], self.currentlys[i], self.buy_prices[i],
                        self.first_bids[i], self.num_bidss[i],
                        self.starteds[i], self.endss[i], self.descs[i])
                f.write(f'{ln}\n')

        self.uids       = []
        self.seller_ids = []
        self.names      = []
        self.currentlys = []
        self.buy_prices = []
        self.first_bids = []
        self.num_bidss  = []
        self.starteds   = []
        self.endss      = []
        self.descs      = []


class Parser:
    def __init__(self):
        self.c = CategoryData()
        self.u = UserData()
        self.b = BidData()
        self.i = ItemData()
        self.ci = CategorizationData()
        
        self.logged = False
        self.nullmap = {}
        self.gnullmap = {}
        self.f = ''
        self.fn = ''

    def _log_if_none(self, key, value, mark):
        assert key is not None
        assert mark is not None

        if value is None:
            if key not in self.gnullmap:
                self.gnullmap[key] = [self.fn]
                self.logged = True
            elif not self.logged:
                self.gnullmap[key].append(self.fn)
                self.logged = True

            log = f'{mark}'
            if key not in self.nullmap:
                self.nullmap[key] = [log]
            else:
                self.nullmap[key].append(log)
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

    def set_file(self, file):
        self.f = os.path.basename(file)
        self.fn = self.f.split('.')[0]
        self.logged = False

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

        target_gnull = os.path.join(reportdir, '0-nullmap.txt')
        with open(target_gnull, 'w+') as f:
            f.write(f'Global Null Map (all files):\n\n')
            f.write(f'These keys were sometimes null when parsing:\n')
            for k in self.gnullmap:
                f.write(f'  {k}\n')
            f.write('\nDetails see below:\n')
            for k in self.gnullmap:
                f.write(f'{k} was null in\n')
                for fn in self.gnullmap[k]:
                    f.write(f'  {fn}\n')
        self.gnullmap = {}

    def report(self):
        target_null = os.path.join(reportdir, f'{self.fn}_nullmap.txt')
        with open(target_null, 'w+') as f:
            f.write(f'Null Map ({self.fn}):\n\n')
            f.write(f'These keys were sometimes null when parsing {self.f}:\n')
            for k in self.nullmap:
                f.write(f'  {k}\n')
            f.write('\nDetails see below (numbers are ItemIDs):\n')
            for k in self.nullmap:
                f.write(f'{k} was null at\n')
                for ln in self.nullmap[k]:
                    f.write(f'  {ln}\n')
        self.nullmap = {}

