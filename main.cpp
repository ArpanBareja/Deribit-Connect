#include <iostream>
#include <string>
#include <curl/curl.h>
#include "include/json.hpp"

using json = nlohmann::json;
using namespace std;

void print_pretty_response(string& response) {
    try {
        json parsed = json::parse(response);
        cout << parsed.dump(4) << endl;  // 4 = indentation level
    } catch (json::parse_error& e) {
        cerr << "Failed to parse JSON: " << e.what() << endl;
        cout << "Raw response: " << response << endl;
    }
}

// Function to handle the response from the cURL request
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// General function to send a cURL request with optional access token
string sendRequest(const string& url, const json& payload, const string& accessToken = "") {
    string readBuffer;
    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);  // Set the HTTP method to POST

        // Set the request payload
        string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        // Set headers, including Authorization if accessToken is provided
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!accessToken.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Request failed: " << curl_easy_strerror(res) << endl;
        }

        // Free Resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

// Function to get the access token
string getAccessToken(const string& clientId, const string& clientSecret) {
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            //{"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"},
            {"client_id", clientId},
            {"client_secret", clientSecret}
        }},
        {"jsonrpc", "2.0"}
    };

    string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    auto responseJson = json::parse(response);

    // Retrieve the access token from the response
    if (responseJson.contains("result") && responseJson["result"].contains("access_token")) {
        return responseJson["result"]["access_token"];
    } else {
        cerr << "Failed to retrieve access token." << endl;
        return "";
    }
}

// Function to place an order
void placeOrder(const string& price, const string& accessToken, const string& amount, const string& instrument) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "private/buy"},
        {"params", {
            {"instrument_name", instrument},
            {"type", "limit"},
            {"price", price},
            {"amount", amount}
        }},
        {"id", 1}
    };

    string response = sendRequest("https://test.deribit.com/api/v2/private/buy", payload, accessToken);
    print_pretty_response(response) ;
}

// Function to cancel an order
void cancelOrder(string &accessToken , string &orderId) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/cancel"},
        {"params", {
            {"order_id", orderId}
        }}
    };

    string response = sendRequest("https://test.deribit.com/api/v2/private/cancel" , payload , accessToken) ;
    print_pretty_response(response) ;
}

// Func to sell Order
void sellOrder(const string& accessToken, const string& price, const string& amount, const string& instrument) {

    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/sell"},
        {"params", {
            {"instrument_name", instrument},
            {"type", "limit"},        // You can change to "market" if needed
            {"price", price},
            {"amount", amount}
        }}
    };

    string url = "https://test.deribit.com/api/v2/private/sell";
    string response = sendRequest(url, payload, accessToken);
    print_pretty_response(response) ;
}

// Func to get Open orders
void getOpenOrders(const string& accessToken) {
    string url = "https://test.deribit.com/api/v2/private/get_open_orders";

    json payload = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "private/get_open_orders"},
        {"params", json::object()}  // Empty object since no params required
    };

    string response = sendRequest(url, payload, accessToken);
    print_pretty_response(response) ;
}

int main() {

    int choice ;
    //client credentials
    string clientId = "vCQ0Y7Yd";
    string clientSecret = "uwpepXfOQUc5C8jTs_JjQcYkbySlm_7XWSaRQ69DlSU";

    // Retrieve the access token
    string accessToken = getAccessToken(clientId, clientSecret);

    if (!accessToken.empty()) {
        
        
        while (true) {
            cout << "--- Deribit Trading Menu ---" << endl;
            cout << "0. Exit" << endl;
            cout << "1. Place Order" << endl;
            cout << "2. Cancel Order" << endl;
            cout << "3. Sell Order" << endl;
            cout << "4. Get Open Orders" << endl;
            cout << "Enter your choice: ";
            cin >> choice;
            
            // modify and  get order book are add ons
            
            switch (choice) {
                case 0:
                    cout << "Exiting..." << endl;
                    return 0; // or break outer loop
                case 1: {
                    string price, amount, instrument;
                    cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                    cin >> instrument;
                    cout << "Enter amount: ";
                    cin >> amount;
                    cout << "Enter price: ";
                    cin >> price;
                    
                    placeOrder(price, accessToken, amount, instrument);
                    break;
                }
                case 2: {
                    string orderId;
                    cout << "Enter Order ID to cancel: ";
                    cin >> orderId;
                    cancelOrder(accessToken, orderId);
                    break;
                }
                
                case 3: {
                    string price, amount, instrument;
                    cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                    cin >> instrument;
                    cout << "Enter amount: ";
                    cin >> amount;
                    cout << "Enter price: ";
                    cin >> price;

                    sellOrder(accessToken , price , amount , instrument) ;
                    break ;
                }
                case 4:
                    getOpenOrders(accessToken);
                    break;

                default:
                    cout << "Invalid choice." << endl;
                    break;
            }
        }
        return 0;
    }
}
