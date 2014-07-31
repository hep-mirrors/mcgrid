import sqlite3
import sys

# Fetch process DB
procDB=sys.argv[1]
sql_conn = sqlite3.connect(procDB)
sql_cur = sql_conn.cursor()

# query
query="SELECT * FROM path;"
sql_cur.execute(query)

# Write out packed files
for idx, row in enumerate(sql_cur):

    name = row[0]
    cont = row[1]

    flname = "{0}".format(name)
    output = open(flname, "wt")
    output.write(str(cont))

    output.close()
