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
        if name is None:
            raise ValueError('DEBUG: The category\'s name must not be null')

        safername = saferstr(name)
        if safername not in self.names:
            self.uids.append(self.next_uid())
            self.names.append(safername)

    def lineify(self):
        return [CategoryData._ln.format(uid, name) 
                for uid, name in zip(self.uids, self.names)]


class UserData:
    _ln = '{}|{}|{}|{}'

    def __init__(self):
        self.uids      = []
        self.ratings   = []
        self.locations = []
        self.countries = []

    def __str__(self):
        return str(self.lineify())

    def add_if_absent(self, uid, rating, location, country):
        if uid is None:
            raise ValueError('DEBUG: The user\'s id must not be null')

        if rating is None:
            raise ValueError('DEBUG: The user\'s rating must not be null')

        if uid not in self.uids:
            saferloc = saferstr(location if location is not None else 'NULL')
            safercntry = saferstr(country if country is not None else 'NULL')

            self.uids.append(saferstr(uid))
            self.ratings.append(rating)
            self.locations.append(saferloc)
            self.countries.append(safercntry)

    def lineify(self):
        return [UserData._ln.format(uid, rating, location, country)
                for uid, rating, location, country
                in zip(self.uids, self.ratings, self.locations, self.countries)]


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
        if user_id is None:
            raise ValueError('DEBUG: The bidder id must not be null')

        if item_id is None:
            raise ValueError('DEBUG: The bidding item id must not be null')

        if time is None:
            raise ValueError('DEBUG: The bid time must not be null')

        if amount is None:
            raise ValueError('DEBUG: The bid amount must not be null')

        self.uids.append(self.next_uid())
        self.user_ids.append(saferstr(user_id))
        self.item_ids.append(item_id)
        self.times.append(saferstr(time))
        self.amounts.append(saferstr(amount))

    def lineify(self):
        return [BidData._ln.format(uid, user_id, item_id, time, amount)
                for uid, user_id, item_id, time, amount
                in zip(self.uids, self.user_ids, self.item_ids, self.times,
                    self.amounts)]


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

    def add(self, item_id, cate_id):
        if item_id is None:
            raise ValueError('DEBUG: The item id must not be null')

        if cate_id is None:
            raise ValueError('DEBUG: The category id must not be null')

        cates = [self.cate_ids[i] for i in range(len(self.item_ids))
                if self.item_ids[i] == item_id]
        if cate_id in cates:
            return

        self.item_ids.append(item_id)
        self.cate_ids.append(cate_id)

    def lineify(self):
        return [CategorizationData._ln.format(uid, item_id, cate_id)
                for uid, item_id, cate_id 
                in zip(self.uids, self.item_ids, self.cate_ids)]


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
        self.ends       = []
        self.descs      = []

    def __str__(self):
        return str(self.lineify())

    def add_if_absent(self, uid, seller_id, name, currently, buy_price,
            first_bid, num_bids, started, end, desc):
        if uid is None:
            raise ValueError('DEBUG: The item id must not be null')

        if seller_id is None:
            raise ValueError('DEBUG: The item\'s seller id must not be null')

        if name is None:
            raise ValueError('DEBUG: The item name must not be null')

        if currently is None:
            raise ValueError('DEBUG: The item currently must not be null')

        if first_bid is None:
            raise ValueError('DEBUG: The item first bid must not be null')

        if num_bids is None:
            raise ValueError('DEBUG: The item number of bids must not be null')

        if started is None:
            raise ValueError('DEBUG: The item start time must not be null')

        if end is None:
            raise ValueError('DEBUG: The item end time must not be null')

        if uid not in self.uids:
            saferbp = saferstr(buy_price if buy_price is not None else 'NULL')

            self.uids.append(uid)
            self.seller_ids.append(saferstr(seller_id))
            self.names.append(saferstr(name))
            self.currentlys.append(saferstr(currently))
            self.buy_prices.append(saferbp)
            self.first_bids.append(saferstr(first_bid))
            self.num_bidss.append(saferstr(num_bids))
            self.starteds.append(saferstr(started))
            self.ends.append(saferstr(end))
            self.descs.append(saferstr(desc))
    
    def lineify(self):
        return [ItemData._ln.format(uid, seller_id, name, currently, buy_price,
                                    first_bid, num_bids, started, end, desc)
                for uid, seller_id, name, currently, buy_price, first_bid,
                    num_bids, started, end, desc
                in zip(self.uids, self.seller_ids, self.names, self.currentlys,
                    self.buy_prices, self.first_bids, self.num_bidss,
                    self.starteds, self.ends, self.descs)]
