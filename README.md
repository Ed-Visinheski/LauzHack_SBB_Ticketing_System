# LauzHack_SBB_Ticketing_System
Anonymous Ticketing System solution for SBB LauzHack Hackathon
# SSB Ticketing System

# App Features

The apps will have 3 tabs. Home/Search, Your Tickets and your Personal Identification Token (PIT)

- Home/Search: Journey Departure and Destination, with date and time selection.
    - This will be a mock version of a train time-table. Fake departure and destination cities by simply always creating a train departure every 30 minutes. Give a fake length of 1 hour? (might not be necessary)
- Your Tickets
    - This will hold all your tickets, with their STATIC QR codes (they will not change) and can be used multiple times.
    - This sections will not hold any of the user’s personal details. Once the ticket is scanned on the inspector’s app it will only display the departure time, the specific train, arrival time and a section telling the inspector to verify the user
- Personal Identification Token
    - This will be a QR code of your Personal Identification Token. This token will be generated from the user’s private key. It can be verified by using the user’s public key to determine if it actually is them.
    - The Ticket inspector will scan your Personal Identification Token (PIT) on their app. This will display if it is the SAME user that purchased the ticket or not. If the PIT is NOT valid, the inspector will know that this is not the user whom purchased the ticket and can issue a fine.

# What will be Required For this Project:

- An application for the User
- An application for the Ticket Inspector
- Users can only be logged in to the system at 1 device at a time - a verification file will be stored locally
- PGP Encryption libraries for C++
- PGP to QR Code converter library for C++
- QR Code image file input for Ticket Inspector (will not be using Camera as POC)
- **Why RNP Makes Sense**
    - The project requires generating a PIT derived from user private keys.
    - Inspectors must verify PITs using matching public keys.
    - RNP supports these OpenPGP digital signature workflows natively.
    - Provides a reliable, standards-compliant method to prevent token forgery and validate user identity during ticket inspection.

# Things to be aware of:

- Once another device logs in and the original is logged out, an email notification should be sent to the user!

# What will be IGNORED

- Payment system can be ignored for this project
- Train timetables - maybe API can be used but a fake one is sufficient
- Camera QR Code Input will be ignored (as it is a POC)

# Tools Required for Implementation

- Core C++
- User App GUI (Qt Library)
- Inspector App GUI (Qt Library)
- QR encode/decode**:** ZXing-C++ (or QZXing if you like Qt integration)
- 
- **PGP Encryption Library (RNP)**
    - We will use **RNP (OpenPGP RNP Library)** as the PGP implementation for C++.
    - RNP provides:
        - Public/private key generation
        - Digital signatures and verification
        - OpenPGP-compatible key formats
        - Lightweight and cross-platform support ideal for Qt-based apps
    - The library is modern, actively maintained, and used in production applications such as Mozilla Thunderbird.
    - RNP enables secure creation and verification of the **Personal Identification Token (PIT)** using the user’s private key and inspector’s public key.