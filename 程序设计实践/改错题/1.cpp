#include <iostream>
#include <memory>
#include <string>
using namespace std;

class Book {
protected:
    string title;
    string author;
    double price;
public:
    Book(string t, string a, double p) : title(t), author(a), price(p) {}
    virtual void printInfo() const {
        cout << "Book: " << title << " by " << author << ", $" << price << endl;
    }
    double getPrice() const { return price; }
};

class EBook : public Book {
private:
    int fileSize;
public:
    EBook(string t, string a, double p, int size)
        : Book(t, a, p), fileSize(size) {}
    void printInfo() const override {
        cout << "E-Book: " << title << " by " << author
            << ", $" << price << ", size: " << fileSize << "MB" << endl;
    }
};

template<typename T>
class Array {
private:
    T* data;
    int size;
public:
    Array(int n) : size(n) {
        data = new T[size];
    }
    ~Array() { delete[] data; }
    T& operator[](int i) { return data[i]; }
    int getSize() const { return size; }
};

template<typename T>
T findMax(T* arr, int n) {
    T maxVal = arr[0];
    for (int i = 1; i < n; i++) {
        if (arr[i] < maxVal) {
            maxVal = arr[i];
        }
    }
    return maxVal;
}

class Library {
private:
    Array<shared_ptr<Book>> books;
    int count;
public:
    Library(int n) : books(n), count(0) {}

    void addBook(shared_ptr<Book> b) {
        if (count < books.getSize()) {
            books[count++] = b;
        }
    }

    void sortByPrice() {
        for (int i = 0; i < count; i++) {
            for (int j = 0; j < count; j++) {
                if (books[j]->getPrice() < books[j + 1]->getPrice()) {
                    swap(books[j], books[j + 1]);
                }
            }
        }
    }

    void printBooks() {
        for (int i = 0; i < count; i++) {
            books[i]->printInfo();
        }
    }

    Array<shared_ptr<Book>>& getBooks() { return books; }
    int getCount() const { return count; }
};

class User {
private:
    string name;
public:
    User(string n) : name(n) {}
    void borrowBook(Library& lib, int index) {
        if (index < lib.getCount()) {
            cout << name << " borrowed: ";
            lib.getBooks()[index]->printInfo();
            lib.getBooks()[index] = nullptr;
        }
    }
};

int main() {
    Library lib(5);
    lib.addBook(make_shared<Book>("C++ Primer", "Lippman", 58.0));
    lib.addBook(make_shared<EBook>("Effective C++", "Meyers", 45.0, 5));
    lib.addBook(make_shared<Book>("STL Source", "Hou Jie", 68.0));

    cout << "Books before sorting:" << endl;
    lib.printBooks();

    cout << endl << "Books after sorting:" << endl;
    lib.sortByPrice();
    lib.printBooks();

    cout << endl;
    User alice("Alice");
    alice.borrowBook(lib, 1);

    // 使用 findMax 查找最高价格
    double prices[] = { 58.0, 45.0, 68.0 };
    double maxPrice = findMax(prices, 3);
    cout << "Max price: " << maxPrice << endl;

    return 0;
}