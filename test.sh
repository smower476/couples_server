# Add user
curl -X POST http://localhost:8080/add-user      -d "username=testuserz"       -d "password=123asdas1324"

# Login (returns jwt token)
curl -X POST http://localhost:8080/login -d "username=testuserz" -d "password=123asdas1324"
