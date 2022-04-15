def betterstr(string):
    """This is a util function that makes a normal string safer.

    This function will first replace every double quotation mark in the
    original string with two double quotation marks, i.e., escapting the double
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
            raise ValueError('The category\'s name must not be null')

        bettername = betterstr(name)
        if bettername not in self.names:
            self.uids.append(self.next_uid())
            self.names.append(bettername)

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
            raise ValueError('The user\'s id must not be null')

        if rating is None:
            raise ValueError('The user\'s rating must not be null')

        if uid not in self.uids:
            betterloc = betterstr(location if location is not None else 'NULL')
            bettercntry = betterstr(country if country is not None else 'NULL')

            self.uids.append(betterstr(uid))
            self.ratings.append(rating)
            self.locations.append(betterloc)
            self.countries.append(bettercntry)

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
        self.uids.append(self.next_uid())
        self.user_ids.append(betterstr(user_id))
        self.item_ids.append(item_id)
        self.times.append(betterstr(time))
        self.amounts.append(betterstr(amount))

    def lineify(self):
        return [BidData._ln.format(uid, user_id, item_id, time, amount)
                for uid, user_id, item_id, time, amount
                in zip(self.uids, self.user_ids, self.item_ids, self.times,
                    self.amounts)]
