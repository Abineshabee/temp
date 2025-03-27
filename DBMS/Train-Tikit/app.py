from cs50 import SQL
from flask import Flask, flash, redirect, render_template, request, session, make_response
from flask_session import Session
from werkzeug.security import check_password_hash, generate_password_hash

# to generate random number to allot seat
import random



# For time representation (HH:MMM:SS) => ()
from datetime import datetime, timedelta

from helpers import login_required

# Configure application
app = Flask(__name__)


# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///database.db")


@app.after_request
def after_request(response):
    """Ensure responses aren't cached"""
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


@app.route("/")
@login_required
def index():
    """Show active bookings"""
    user = session["user_id"]

    # Getting tickets by user and should be of future journey
    tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked' AND journey_date > CURRENT_DATE", user)

    # Returning index template with active bookings
    return render_template("index.html", tickets = tickets)


@app.route("/book", methods=["GET", "POST"])
@login_required
def book():
    # POST method
    if request.method == "POST":
        (src, dest, train, name, date) = (request.form.get("src"), request.form.get("dest"), request.form.get("train"), request.form.get("name"), request.form.get("date"))
        # Getting station list for options in select
        stations = db.execute("SELECT * FROM stations;")
        # If source not selected
        if not src:
            flash("Select Source Station")
            return render_template("book.html", stations=stations)

        # Id destination not selected
        if not dest:
            flash("Select Destination Station")
            return render_template("book.html", stations=stations)

        # If train not selected
        trains = db.execute("SELECT src.train_no AS no, src.train_name AS name FROM ( SELECT train_no, train_name, departure_time, station_name, distance FROM schedule WHERE station_code=? ) src JOIN ( SELECT train_no, train_name, arrival_time, station_name, distance FROM schedule WHERE station_code=? ) dest ON dest.train_no=src.train_no AND dest.distance-src.distance > 0; ",src,dest)
        if not train:
            # Getting trains from src->dest

            flash("Please Select Train From List")
            return render_template("book.html",src=src, dest=dest, trains=trains, allowedDate=[datetime.now().date() + timedelta(days=10), datetime.now().date()])

        # If Name not entered
        if not name:
            flash("Please enter passenger name")
            return render_template("book.html",src=src, dest=dest, trains=trains, allowedDate=[datetime.now().date() + timedelta(days=10), datetime.now().date()])

        # If date not choosen
        if not date:
            flash("Please select journey date")
            return render_template("book.html",src=src, dest=dest, trains=trains, allowedDate=[datetime.now().date() + timedelta(days=10), datetime.now().date()])

        if not (datetime.now().date() <= datetime.strptime(date, "%Y-%m-%d").date() <= datetime.now().date() + timedelta(days=10)):
            flash("Date out of bound")
            return render_template("book.html",src=src, dest=dest, trains=trains, allowedDate=[datetime.now().date() + timedelta(days=10), datetime.now().date()])

        # somehow selection of source station or destination made where tarins doesn't go
        srcDist=db.execute("SELECT distance FROM schedule WHERE train_no=? AND station_code=?",train,src)
        destDist=db.execute("SELECT distance FROM schedule WHERE train_no=? AND station_code=?",train,dest)
        if (not srcDist) or (not destDist) or (destDist[0]['distance'] <= srcDist[0]['distance']):
            flash("INVALID SOURCE STATION OR TRAIN")
            return render_template("book.html",src=src, dest=dest, trains=trains, allowedDate=[datetime.now().date() + timedelta(days=10), datetime.now().date()])


        # Now we have source, destination and train name in which we have to book ticket.
        user = session["user_id"]
        fare = round((destDist[0]['distance']-srcDist[0]['distance']) * 0.6)

        # Getting booked seats
        booked=[]
        rows = db.execute("SELECT seat_no FROM tickets WHERE train_no=?",train)
        for row in rows:
            booked.append(row['seat_no'])
        if len(booked) > 99:
            flash("No more seats left in selected train")
            return render_template("book.html",src=src, dest=dest, trains=trains, allowedDate=[datetime.now().date() + timedelta(days=10), datetime.now().date()])

        # Alloting seat from non-booked seats
        seat = random.randint(1, 100)
        while seat in booked:
            seat = random.randint(1,100)

        # Updating database of current booking
        db.execute("INSERT INTO tickets (userid, journey_date, train_no, board, deboard, passenger, fare, seat_no) VALUES(?, ?, ?, ?, ?, ?, ?, ?)", user, date, train, src, dest, name, fare, seat)
        balance = db.execute("SELECT balance FROM users WHERE id=?", user)[0]['balance']
        db.execute("UPDATE users SET balance=? WHERE id=?", balance - fare, user)

        flash("Ticket Booked Successfully!")

        # Getting tickets by user and should be of future journey
        tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked' AND journey_date > CURRENT_DATE", user)

        # Returning index template with active bookings
        return render_template("index.html", tickets = tickets)

    # GET method
    else:
        # Getting station list for options in select
        stations = db.execute("SELECT * FROM stations;")
        return render_template("book.html", stations=stations)


@app.route("/transactions")
@login_required
def transactions():
    """Show history of transactions"""
    user = session["user_id"]
    transactions = db.execute("SELECT id, booking_date AS transaction_date, journey_date, train_no, board, deboard, fare, status FROM tickets WHERE userid=? AND status='booked' UNION SELECT id, cancelling_date AS transaction_date, journey_date, train_no, board, deboard, fare, status FROM tickets WHERE userid=? AND status='cancelled' ORDER BY transaction_date DESC;", user, user)

    # Fetching balance from users table
    balance = db.execute("SELECT balance FROM users WHERE id=?",user)[0]['balance']

    # Returning template with data to render
    return render_template("transactions.html", transactions = transactions, balance=balance)


@app.route("/login", methods=["GET", "POST"])
def login():
    """Log user in"""

    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":

        # Ensure username was submitted
        if not request.form.get("username"):
            flash("Please Enter Username")
            return render_template("login.html")

        # Ensure password was submitted
        elif not request.form.get("password"):
            flash("Please Enter Password")
            return render_template("login.html")

        # Query database for username
        rows = db.execute("SELECT * FROM users WHERE username = ?", request.form.get("username"))

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(rows[0]["hash"], request.form.get("password")):
            flash("Username and/or password is incorrect.")
            return render_template("login.html")

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    else:
        return render_template("login.html")


@app.route("/logout")
def logout():
    """Log user out"""

    # Forget any user_id
    session.clear()

    # Redirect user to login form
    return redirect("/")


@app.route("/search", methods=["GET", "POST"])
@login_required
def search():
    """Search Trains Between Sations."""
    if request.method == "POST":

        # Getting form data
        (src, dest) = (request.form.get("src"), request.form.get("dest"))

        # If source not selected
        if not src:
            flash("Please Select Source")

            # Stations list
            stations = db.execute("SELECT * FROM stations;")

            return render_template("search.html", stations=stations)

        # If destination not selected
        if not dest:
            flash("Plaese Select Destination")

            # Stations list
            stations = db.execute("SELECT * FROM stations;")

            return render_template("search.html", stations=stations)

        """
        # SQL QUERY to get trains from given src and dest
        "
                SELECT src.train_no, src.train_name, src.departure_time, src.station_name, src.arrival_time, src.station_name, dest.distance - src.distance AS distance
                FROM
                (
                    SELECT train_no, train_name, departure_time, station_name, distance
                    FROM schedule
                    WHERE station_code=?
                ) src
                JOIN
                (
                    SELECT train_no, train_name, arrival_time, station_name, distance
                    FROM schedule
                    WHERE station_code=?
                ) dest
                ON dest.train_no=src.train_no
                AND dest.distance-src.distance > 0;
                "

        """

        # Getting trains from src->dest
        data = db.execute("SELECT src.train_no AS no, src.train_name AS name, src.departure_time AS deptTime, src.station_name AS source, dest.arrival_time AS arrTime, dest.station_name AS destination, dest.distance - src.distance AS distance FROM ( SELECT train_no, train_name, departure_time, station_name, distance FROM schedule WHERE station_code=? ) src JOIN ( SELECT train_no, train_name, arrival_time, station_name, distance FROM schedule WHERE station_code=? ) dest ON dest.train_no=src.train_no AND dest.distance-src.distance > 0; ",src,dest)

        # Converting time to STD format
        for d in data:
            d['deptTime']= datetime.strptime(d['deptTime'][1:-1], '%H:%M:%S').strftime('%-I:%M %p')
            d['arrTime']= datetime.strptime(d['arrTime'][1:-1], '%H:%M:%S').strftime('%-I:%M %p')

        return render_template("trains.html", data=data, src=src, dest=dest)

    else:
        # Stations list
        stations = db.execute("SELECT * FROM stations;")

        return render_template("search.html", stations=stations)


@app.route("/register", methods=["GET", "POST"])
def register():
    """Register user"""
    if request.method == "POST":

        (username, password, confirmation) = (request.form.get("username"), request.form.get("password"), request.form.get("confirmation"))

        # Ensure username not empty
        if not username:
            flash("Please Enter Username")
            return render_template("register.html")

        # Ensure password not empty
        if not password:
            flash("Please Enter Password")
            return render_template("register.html")

        # Ensure confirmation password not empty
        if not confirmation:
            flash("Please Enter Confirmation Password")
            return render_template("register.html")

        # Ensure passord and confirmation is same
        if password != confirmation:
            flash("Password and Confirmation Password are Diffrent")
            return render_template("register.html")

        # Retrieving usernames with same in database
        users = db.execute("SELECT username FROM users WHERE username=?;", username)

        # If user not already present
        if not users and password == confirmation:

            # Generating hash of password
            password = generate_password_hash(password)

            # Storing user in databse
            db.execute("INSERT INTO users (username, hash) VALUES(?,?);", username, password)
            rows = db.execute("SELECT * FROM users WHERE username=?", username)

            session["user_id"] = rows[0]["id"]

            flash("You have registered successfully!")

            # Getting tickets by user and should be of future journey to be rendered on index.html
            tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked' AND journey_date > CURRENT_DATE", session["user_id"])

            # Returning index template with active bookings
            return render_template("index.html", tickets = tickets)

        # If user already present in database
        else:
            flash("Username already exists.")
            return render_template("register.html")
    else:
        return render_template("register.html")


@app.route("/cancel", methods=["GET", "POST"])
@login_required
def cancel():
    """Cancel tickets"""
    user = session["user_id"]
    if request.method == "POST":

        # getting form data
        ticket = request.form.get("ticket")

        # If submitting form without selecting ticket
        if not ticket:
            flash("Please Select Ticket to Cancel")
            tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked'", user)
            return render_template("cancel.html", tickets=tickets)

        # Checking if ticket booked by this user
        t = db.execute("SELECT * FROM tickets WHERE id=? AND status='booked' AND userid=?", ticket, user)

        # not t means ticket is invalid of this user did'nt booked this ticket
        if not t:
            flash("Invalid Ticket")
            tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked'", user)
            return render_template("cancel.html", tickets=tickets)

        # It will have single row if valid ticket found because of unique ticket id
        t = t[0]

        if datetime.strptime(t['journey_date'], "%Y-%m-%d").date() <= datetime.now().date():
            flash("Ticket Can't be cancelled on or later journey date.")
            tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked'", user)
            return render_template("cancel.html", tickets=tickets)

        # Updating status and cancelling_date in tickets table
        db.execute("UPDATE tickets SET status='cancelled', cancelling_date=CURRENT_TIMESTAMP WHERE id=?", ticket)

        # Updating balance in users table
        balance = db.execute("SELECT balance FROM users WHERE id=?", user)[0]["balance"]
        db.execute("UPDATE users SET balance=? WHERE id=?", balance + t['fare'], user)

        flash("Ticket Cancelled")

        tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked'", user)
        return render_template("cancel.html", tickets=tickets)
    else:
        tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked' AND journey_date > DATE('now')", user)
        return render_template("cancel.html", tickets=tickets)


@app.route("/password", methods=["GET", "POST"])
@login_required
def change_password():
    """Change password"""
    if request.method == "POST":

        # Getting user id of requeste user
        user = session["user_id"]

        # Getting form data
        (password, new, confirmation) = (request.form.get("password"), request.form.get("new"), request.form.get("confirmation"))

        # Ensure no empty value
        if (not password) or (not new) or (not confirmation):
            flash("All fields mandatory.")
            return render_template("password.html")

        # New Password and Confirm New Password not same.
        if new != confirmation:
            flash("New Password & Confirm New Password must be same.")
            return render_template("password.html")

        # Query database for username
        rows = db.execute("SELECT * FROM users WHERE id = ?", user)

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(rows[0]['hash'], password):
            flash("Invalid Old Password")
            return render_template("password.html")

        # if old password found correct and both new password are same the we'll update hash field in database
        hash = generate_password_hash(new)
        db.execute("UPDATE users SET hash=? WHERE id=?", hash, user)

        flash("Password Updated Successfully")
        return render_template("password.html")

    else:
        return render_template("password.html")


@app.route("/print", methods=["GET", "POST"])
def print():
    user = session["user_id"]
    if request.method == "POST":
        ticket = request.form.get("ticket")
        if not ticket:
            flash("Please Select a ticket to print.")
            tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked'", user)
            return render_template("print.html", tickets=tickets)


        # Checking if ticket booked by this user
        tickets = db.execute("SELECT * FROM tickets WHERE id=? AND status='booked' AND userid=?", ticket, user)

        # not t means ticket is invalid of this user did'nt booked this ticket
        if not tickets:
            flash("Invalid Ticket!")
            tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked'", user)
            return render_template("print.html", tickets=tickets)

        # It will have single row if valid ticket found because of unique ticket id
        tickets = tickets[0]

        # Getting train name using train_no
        tickets['train_name'] = db.execute("SELECT DISTINCT train_name FROM schedule WHERE train_no=?",tickets['train_no'])[0]['train_name']

        # Getting source and destination station name and code
        stations = db.execute("SELECT DISTINCT source_station_code, source_station_name, destination_station_code, destination_station_name FROM schedule WHERE train_no=?",tickets['train_no'])[0]

        # getting name of boarding station
        stations['boarding'] = db.execute("SELECT DISTINCT station_name FROM schedule WHERE station_code=?",tickets['board'])[0]['station_name']

        # Getting name of deboarding station
        stations['deboarding'] = db.execute("SELECT DISTINCT station_name FROM schedule WHERE station_code=?",tickets['deboard'])[0]['station_name']


        # getting departure & arrival time
        departureTime = db.execute("SELECT departure_time FROM schedule WHERE train_no=? AND station_code=?",tickets['train_no'],tickets['board'])[0]['departure_time'][1:-1]
        arrivalTime = db.execute("SELECT arrival_time FROM schedule WHERE train_no=? AND station_code=?",tickets['train_no'],tickets['deboard'])[0]['arrival_time'][1:-1]

        tickets['departureTime'] = datetime.strptime(departureTime, '%H:%M:%S').strftime('%-I:%M %p')
        tickets['arrivalTime'] = datetime.strptime(arrivalTime, '%H:%M:%S').strftime('%-I:%M %p')

        tickets['booking_date'] = datetime.strptime(tickets['booking_date'], '%Y-%m-%d %H:%M:%S').strftime('%Y-%m-%d %I:%M %p')

        username = db.execute("SELECT username FROM users WHERE id=?",user)[0]['username']

        # html page to show ticket
        return render_template("ticket.html", id = ticket, ticket_data=tickets, stations=stations, user=username)


    else:
        tickets = db.execute("SELECT * FROM tickets WHERE userid=? AND status='booked' AND journey_date > DATE('now')", user)
        return render_template("print.html", tickets=tickets)