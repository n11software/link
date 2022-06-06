let browser;
if (navigator.userAgent.match(/chrome|chromium|crios/i)) browser = "Chrome"
else if (navigator.userAgent.match(/firefox|fxios/i)) browser = "Firefox"
else if (navigator.userAgent.match(/safari/i)) browser = "Safari"
else if (navigator.userAgent.match(/opr\//i)) browser = "Opera"
else if (navigator.userAgent.match(/edg/i)) browser = "Edge"
else browser = "Unknown"
let os = navigator.platform=="Win32"? "Windows": navigator.platform=="MacIntel"? "Mac": navigator.platform=="Linux"? "Linux": navigator.platform=="Android"? "Android": navigator.platform=="iPhone"? "iOS": navigator.platform=="iPad"? "iOS": navigator.platform=="iPod"? "iOS": "Unknown"
let Request = new XMLHttpRequest()
let Emails = []
let submit = () => {
  if (document.getElementById("email").value == "" && document.getElementById("password").value == "") {
    document.getElementById("error").innerHTML = "Please fill in all fields"
    document.getElementById("email-wrapper").style.border = "1px solid red"
    document.getElementById("password-wrapper").style.border = "1px solid red"
    document.getElementById("email-wrapper").focus()
  } else if (document.getElementById("email").value == "") {
    document.getElementById("error").innerHTML = "Please fill in all fields"
    document.getElementById("email-wrapper").style.border = "1px solid red"
    document.getElementById("email-wrapper").focus()
  } else if (document.getElementById("password").value == "") {
    document.getElementById("error").innerHTML = "Please fill in all fields"
    document.getElementById("password-wrapper").style.border = "1px solid red"
    document.getElementById("email-wrapper").style.border = "1px solid #ccc"
    document.getElementById("password-wrapper").focus()
  } else {
    if (Emails.includes(document.getElementById("email").value)) {
      document.getElementById("error").innerHTML = "You are already logged in with this email!"
      document.getElementById("email-wrapper").style.border = "1px solid red"
      document.getElementById("email-wrapper").focus()
    } else {
      document.getElementById("error").innerHTML = ""
      document.getElementById("email-wrapper").style.border = "1px solid #ccc"
      document.getElementById("password-wrapper").style.border = "1px solid #ccc"
      Request.open("POST", "/api/login", true)
      Request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
      Request.send("email="+document.getElementById("email").value+"&password="+document.getElementById("password").value+"&browser="+browser+"&os="+os)
    }
  }
}

document.getElementById("login").onclick = submit
document.getElementById("email").onkeydown = (e) => { if (e.keyCode == 13) submit() }
document.getElementById("password").onkeydown = (e) => { if (e.keyCode == 13) submit() }
Request.onreadystatechange = () => {
  if (Request.readyState == 4) {
    let Response = JSON.parse(Request.responseText)
    if (Response.Token == undefined) {
      document.getElementById("error").innerHTML = Response.error
      if (Response.error == "Wrong email or password") {
        document.getElementById("email-wrapper").style.border = "1px solid red"
        document.getElementById("password-wrapper").style.border = "1px solid red"
      }
    } else {
      localStorage.getItem("Tokens") == null? localStorage.setItem("Tokens", 1): localStorage.setItem("Tokens", (parseInt(localStorage.getItem("Tokens"))+1))
      localStorage.setItem("Token-"+localStorage.getItem("Tokens"), Response.Token)
      localStorage.setItem("CurrentToken", localStorage.getItem("Tokens"))
      CheckEmails()
      window.location.href = "/"
    }
  }
}
let CheckEmails = () => {
  Emails = []
  let Check = new XMLHttpRequest()
  let Tokens = localStorage.getItem("Tokens")
  if (Tokens != null) {
    for (let i=1;i<=Tokens;i++) {
      Check.open("POST", "/api/user", true)
      Check.setRequestHeader("Authorization", "Bearer "+localStorage.getItem("Token-"+i))
      Check.send()
    }
  }

  Check.onreadystatechange = () => {
    if (Check.readyState == 4) {
      let Response = JSON.parse(Check.responseText)
      if (Response.error == undefined) {
        Emails.push(Response.email)
      }
    }
  }
}
CheckEmails()