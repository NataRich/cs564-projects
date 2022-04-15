"""models.py (tentative)

This file contains object representations of the schema relations. But note
that these objects are persistent objects that directly interact with database
and should not be used to parse the JSON data.

To parse the JSON data, see data_models.py.
"""

class Item:
    """Item Entity

    This class represents the table that stores information about an item.

    In the JSON file, an Item instance has extra attributes, such as an array
    of categories, an array of bids, etc. These attributes will not be stored
    in the item entity but can be fetched in conjunction with other tables.

    Each item has only one seller, so `seller_id` must not be null. But since
    a seller can sell many items, `seller_id` is not necessarily unique. Also,
    since the seller of an item decides whether to set a buy price, `buy_price`
    can be null if they decide not to set one.

    Attributes:
        uid (int)       : PK.
        seller_id (str) : FK. The seller's uid.
        name (str)      : A short item description.
        currently (str) : The current highest bid.
        buy_price (str) : The price, set before the auction, at which the
                          bidder can win the auction immediately. 
        first_bid (str) : The minimum qualifying first-bid amount.   
        num_bids (int)  : Number of bids.
        started (str)   : Auction start time. 
        ends (str)      : Auction end time.
        desc (str)      : A long item description.

    Notes on attributes:
        seller_id : NOT NULL
        buy_price : NULLABLE
    """
    def __init__(self, uid, seller_id, name, currently, buy_price, first_bid,
            num_bids, location, country, started, ends, desc):
        self.uid       = uid
        self.seller_id = seller_id
        self.name      = name
        self.currently = currently
        self.buy_price = buy_price
        self.first_bid = first_bid
        self.num_bids  = num_bids
        self.started   = started
        self.ends      = ends
        self.desc      = desc


class User:
    """User Entity

    This class represents the table that stores information about a user.

    Attributes:
        uid (str)      : PK.
        rating (int)   : User's rating.
        location (str) : User's location.
        country (str)  : User's country.

    Notes on attributes:
        location : NULLABLE
        country  : NULLABLE
    """
    def __init__(self, uid, rating, location, country):
        self.uid      = uid
        self.rating   = rating
        self.location = location
        self.country  = country


class Category:
    """Category Entity

    This class represents the table that stores information about a category.

    This entity is necessary despite only two fields are required because
    storing strings in the database is expensive. In particular, categories
    are used extensively and repeatedly in the schema. Therefore, it's a better
    idea to store just an integer value as the reference to some category.

    Attributes:
        uid (int)  : PK.
        name (str) : A short description of the category.

    Notes on attributesi:
        uid  : AUTO INCREMENT
        name : UNIQUE NOT NULL
    """
    def __init__(self, uid, name):
        self.uid  = uid
        self.name = name


class Bid:
    """Bid Entity

    This class represents the table that stores information about a bid.

    In the JSON file, a Bid instance should display some information about the
    bidder, but that is hidden in the database since it makes more sense to
    store a reference to the user and merge the data needed during fetching.

    Each item should be able to have multiple bid records (i.e., Bid
    instances), so the attribute `item_id` is not unique in this table.
    Similarly, each user should be able to bid multiple items, so the `user_id`
    is not unique.

    Attributes:
        uid (int)     : PK.
        user_id (str) : FK. The bidder's uid.
        item_id (int) : FK. The item's uid.
        time (str)    : The time when the bidder offers the price.
        amount (str)  : The price the bidder offers.

    Notes on attributes:
        uid     : AUTO INCREMENT
        user_id : NOT NULL
        item_id : NOT NULL
        time    : NOT NULL
        amount  : NOT NULL
    """
    def __init__(self, uid, user_id, item_id, time, amount):
        self.uid     = uid
        self.user_id = user_id
        self.item_id = item_id
        self.time    = time
        self.amount  = amount


class Categorization:
    """Categorization Entity

    This class represents the junction table that associates items with
    a category or many categories.

    Each item must have at least one category, which unfortunately cannot be
    checked by the properties of this table. Since each item can belong to many
    categories, `item_id` is not unique. Also, since each category can contain
    many items, `cate_id` is not unique, either.

    Attributes:
        uid (int)     : PK.
        item_id (int) : FK. The item's uid.
        cate_id (int) : FK. The category's uid.

    Notes on attributes:
        uid     : AUTO INCREMENT
        item_id : NOT NULL
        cate_id : NOT NULL
    """
    def __init__(self, uid, item_id, cate_id):
        self.uid     = uid
        self.item_id = item_id
        self.cate_id = cate_id
