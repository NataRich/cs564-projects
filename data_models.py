safernull = '"NULL"'


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