let AddContent = () => {
  console.log(LoggedIn)
  if (LoggedIn) {
    let ProfilePicture = document.createElement("a")
    ProfilePicture.innerHTML = "<img src='/api/user/pfp?email="+Email+"'>"
    ProfilePicture.className = "pfp"
    ProfilePicture.onclick = () => {
      document.getElementById("users").classList.toggle("hidden")
    }
    document.body.prepend(ProfilePicture)
    RegisterAll(()=>{
      for (let i=0;i<UUIDs.length;i++) {
        let User = document.createElement("div")
        User.className = "user"
        User.innerHTML = "<img src='/api/user/pfp?email="+Emails[i]+"'/>\n"
        User.innerHTML += "<span>" + Emails[i] + "</span>"
        User.onclick = () => {
          window.location.href = "/user?uuid="+UUIDs[i]
        }
        document.getElementById("users").appendChild(User)
      }
    })
    
  } else {
    let LoginButton = document.createElement("a")
    LoginButton.className = "login"
    LoginButton.innerHTML = "Login"
    LoginButton.href = "/login"
    document.body.prepend(LoginButton)
  }
}

console.log(LoggedIn)
if (LoggedIn == undefined) {
  Register(()=>{
    AddContent()
  })
} else {
  AddContent()
}