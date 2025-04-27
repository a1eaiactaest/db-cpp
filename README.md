# Database
## Notes before
### Requirements
Support for:
- (DDL) create, delete database structures, allow for modification of those structures (add column to the table).
- (DML) manipulate data in the structures for data input.
- (DQL) at least two query operations (`SELECT`, `WHERE`).

Database must support composing the operations, user should be able to use any amount of the `WHERE` operation in one query.

Additionally, create a support for the operations saving and reading DB state from the file - form of the backup. It should be able to save and read from many files specified by the user.

Database should enforce data type safety.

Operations should not modify the backup file every time (as this significantly impacts performance). It's best to keep the data in memory and only occasionally (e.g., on demand or when closing the application) save its state to a file.
