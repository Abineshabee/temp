# Train Tikit
## Live link: https://train-tikit.onrender.com
## Description:
### This is an Online Rail Ticket Booking App.

### This Project is inspired by Currently running Indian Railways Online Ticket Booking App [IRCTC](irctc.co.in/) by Railway Ministry Govt of India **I just tried making something like that this is little clone of that application**.

### We can Search trains between two stations
#### On searching it will give us list of train numbers, with names running between given station along with fare, distance, departure and arrival times.

### We can Book tickets between two stations
#### Initially at `/book` route we will have form with two fields Source station selector and destination station selector.
#### We have Fetch button clicking on which will add three more fields:
+ Train selector with list of trains between selected stations,
+ Passenger Full Name,
+ Date of journey
#### after filling all inputs ticket will books if details found valid other wise form returned with error, if okay redirected to `index` route

### We can Cancel tickets booked by user
#### On `/cancel` route we will have a selector options with active tickets booked by this user After selecting ticket and then clicking cancel button will submit this form and Ticket will cancelled and returned with succesfull message or unsuccessfull of invalid input.

### We can print tickets booked by user
#### On `/print` route we will have same interface as cancel but having print button instead of cancel clicking on which will go to get the pdf of our ticket and can be downloaded to local storage.

### We can see transactionn history
#### We can see only active bookings at `index` route but we can here see all transactions made by user either cancelled ticket or booked a ticket with Time of transaction, Transaction Type, Amount of transaction, ticket details on which transaction made and at top left Wallet Balance left in user account is shown.

### And other authentication loggings are:
+ #### Change Password
+ #### Logout
+ #### Register
+ #### Login

### Some assumptions:
+ assumed that there are only 100 seats in train
+ assumed Fare will be `0.6*distance`
+ assumed train type of all are identical &  no classes
+ schedule of trains were only correct till 2017 as this data provided by govt. is of year 2017.
+ data taken from [Open Govenrment Data Govt. of India](https://data.gov.in/)

## In future this application can be upgraded as
+ Booking for multiple passengers at single ticket
+ Payment Gateway Integration Can be made
+ UI Improvements
+ Users can choose which seat to book from vacant seats instead of random allotment
+ Can be used to book buses, or others


## Runnig Flask Application:
```
pip install -r requirements.txt
flask run
```


## Technologies Used:
+ Flask - Python Framework for Web Development
+ HTML - Basic Structure & Skeletal of Web Pages
+ CSS - Styled the HTML Elements
+ Bootstrap - Framework for Front-end Web Development

### Modules used
+ **cs50** - cs50's API to operate SQL database
+ **Flask** - Whole application is made with Flask Frame work
+ **weasyprint** - python module to make pdf



## Implementation:
### Routes:
+ `/index` - shows current active booking **Login Required**
+ `/search` - search trains between two stations **Login Required**
+ `/book` - book ticket required input fields source station, destination, train, passenger name and date of journey **Login Required**
+ `/cancel` - options will show active bookings by user to select and cancel **Login Required**
+ `/transaction` - this route will show transaction history by transaction data **Login Require**
+ `/print` - options for active bookings will shown, user can select and print this ticket as in pdf file **Login Required**
+ `/logout` - this route will clear session
+ `/login` - this will return form though which we can login
+ `/register` - this will return form thought which we can register for new user

### Templates:
+ **book** - Form to book fields are source station, destination station, train no&name, passenger name, date of journey
+ **cancel** - Form with select type having options of active ticket instances
+ **index** - Home page with a table showing active bookings
+ **layout** - This is basic structure used by almost all templates, it's code inherited by all templates
+ **login** - This template have login form
+ **password** - This template have form for password updation
+ **print** - This will show form having ticket selector to print
+ **register** - This template will show form for registration of user
+ **search** - This will show form having inputs source station and destination station to search trains between them
+ **ticket** - This template is used to make pdf
+ **trains** - This will render list* of trains From and To stations
+ **transations** - This will show all transactiona made by user either is cancelled or booked

### Python Files:
+ **app.py** - main flask application all routes are defined in this file
+ **helpers.py** - this file contains only one fucntion login_required this file is taken from previous flask application assigned by cs50.
+ **requirements.txt** - this text file contains only names of python modules need to be installed before running Flask application
+ **database.db** - this is SQL database having tables: schedules, stations, trains, users and tickets
+ **static** folder contains css-stylesheet, favicon.ico and logo.png
