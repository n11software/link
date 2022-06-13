let browser;
if (navigator.userAgent.match(/chrome|chromium|crios/i)) browser = "Chrome"
else if (navigator.userAgent.match(/firefox|fxios/i)) browser = "Firefox"
else if (navigator.userAgent.match(/safari/i)) browser = "Safari"
else if (navigator.userAgent.match(/opr\//i)) browser = "Opera"
else if (navigator.userAgent.match(/edg/i)) browser = "Edge"
else browser = "Unknown"
let os = navigator.platform=="Win32"? "Windows": navigator.platform=="MacIntel"? "Mac": navigator.platform=="Linux"? "Linux": navigator.platform=="Android"? "Android": navigator.platform=="iPhone"? "iOS": navigator.platform=="iPad"? "iOS": navigator.platform=="iPod"? "iOS": "Unknown"
// let Request = new XMLHttpRequest()
// let Emails = []
// let submit = () => {
//   if (document.getElementById("email").value == "" && document.getElementById("password").value == "") {
//     document.getElementById("error").innerHTML = "Please fill in all fields"
//     document.getElementById("email-wrapper").style.border = "1px solid red"
//     document.getElementById("password-wrapper").style.border = "1px solid red"
//     document.getElementById("email-wrapper").focus()
//   } else if (document.getElementById("email").value == "") {
//     document.getElementById("error").innerHTML = "Please fill in all fields"
//     document.getElementById("email-wrapper").style.border = "1px solid red"
//     document.getElementById("email-wrapper").focus()
//   } else if (document.getElementById("password").value == "") {
//     document.getElementById("error").innerHTML = "Please fill in all fields"
//     document.getElementById("password-wrapper").style.border = "1px solid red"
//     document.getElementById("email-wrapper").style.border = "1px solid #ccc"
//     document.getElementById("password-wrapper").focus()
//   } else {
//     if (Emails.includes(document.getElementById("email").value)) {
//       document.getElementById("error").innerHTML = "You are already logged in with this email!"
//       document.getElementById("email-wrapper").style.border = "1px solid red"
//       document.getElementById("email-wrapper").focus()
//     } else {
//       document.getElementById("error").innerHTML = ""
//       document.getElementById("email-wrapper").style.border = "1px solid #ccc"
//       document.getElementById("password-wrapper").style.border = "1px solid #ccc"
//       Request.open("POST", "/api/login", true)
//       Request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
//       Request.send("email="+document.getElementById("email").value+"&password="+document.getElementById("password").value+"&browser="+browser+"&os="+os)
//     }
//   }
// }

// document.getElementById("login").onclick = submit
// document.getElementById("email").onkeydown = (e) => { if (e.keyCode == 13) submit() }
// document.getElementById("password").onkeydown = (e) => { if (e.keyCode == 13) submit() }
// Request.onreadystatechange = () => {
//   if (Request.readyState == 4) {
//     let Response = JSON.parse(Request.responseText)
//     if (Response.Token == undefined) {
//       document.getElementById("error").innerHTML = Response.error
//       if (Response.error == "Wrong email or password") {
//         document.getElementById("email-wrapper").style.border = "1px solid red"
//         document.getElementById("password-wrapper").style.border = "1px solid red"
//       }
//     } else {
//       localStorage.getItem("Tokens") == null? localStorage.setItem("Tokens", 1): localStorage.setItem("Tokens", (parseInt(localStorage.getItem("Tokens"))+1))
//       localStorage.setItem("Token-"+localStorage.getItem("Tokens"), Response.Token)
//       localStorage.setItem("CurrentToken", localStorage.getItem("Tokens"))
//       CheckEmails()
//       window.location.href = "/"
//     }
//   }
// }
// let CheckEmails = () => {
//   Emails = []
//   let Check = new XMLHttpRequest()
//   let Tokens = localStorage.getItem("Tokens")
//   if (Tokens != null) {
//     for (let i=1;i<=Tokens;i++) {
//       Check.open("POST", "/api/user", true)
//       Check.setRequestHeader("Authorization", "Bearer "+localStorage.getItem("Token-"+i))
//       Check.send()
//     }
//   }

//   Check.onreadystatechange = () => {
//     if (Check.readyState == 4) {
//       let Response = JSON.parse(Check.responseText)
//       if (Response.error == undefined) {
//         Emails.push(Response.email)
//       }
//     }
//   }
// }
// CheckEmails()











// New login.js variables
let CodeFAType
let CodeFAID
let CodeFARetry = false
let Email

// New login.js - Account Selector
let LoginPreExisting = i => {
  window.location.href = "/u/"+i+"/"
}

document.querySelector("#account-select>.button").onclick = () => {
  document.getElementById("account-select").classList.toggle("hidden")
  document.getElementById("email").classList.toggle("hidden")
  document.querySelector("#email>.input>input").focus()
}

// New login.js - Email Input
let ErrorEmail = () => {
  document.querySelector(".input>svg.error").classList.remove("hidden")
  document.querySelector(".input>svg.success").classList.add("hidden")
  document.querySelector("#email>a.button").setAttribute("disabled", "")
}

let SuccessEmail = () => {
  document.querySelector(".input>svg.error").classList.add("hidden")
  document.querySelector(".input>svg.success").classList.remove("hidden")
  document.querySelector("#email>a.button").removeAttribute("disabled")
}

document.getElementById("email").onkeyup = () => {
  let val = document.querySelector("#email>.input>input").value
  Email = val
  if (val.length>0 && /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(val)) SuccessEmail()
  else ErrorEmail()
}

let EmailNext = () => {
  let CheckEmail = new XMLHttpRequest()
  CheckEmail.open("GET", "/api/user/exists?email="+Email, true)
  CheckEmail.send()
  CheckEmail.onreadystatechange = () => {
    if (CheckEmail.readyState == 4 && document.querySelector("#email>.input>input").value == Email && CheckEmail.responseText == "{\"exists\":true}") {
      document.getElementById("email").classList.toggle("hidden")
      document.getElementById("password").classList.toggle("hidden")
      document.querySelector("#email>a.button").removeAttribute("disabled")
      document.querySelector(".input>svg.error").classList.add("hidden")
      document.querySelector(".input>svg.success").classList.remove("hidden")
      document.querySelector("#password>.input>input").focus()
    } else {
      document.querySelector(".input>svg.error").classList.remove("hidden")
      document.querySelector(".input>svg.success").classList.add("hidden")
      document.querySelector("#email>a.button").setAttribute("disabled", "")
    }
  }
}

document.querySelector("#email>.input>input").onkeydown = (e) => { if (e.keyCode == 13) EmailNext() }
document.querySelector("#email>a.button").onclick = () => EmailNext()

// New login.js - Password Input
document.querySelector("#password>div.buttons>a.button-secondary").onclick = () => {
  document.getElementById("password").classList.toggle("hidden")
  document.getElementById("email").classList.toggle("hidden")
}

document.querySelector("#password>.input>input").onkeyup = (e) => {
  let val = document.querySelector("#password>.input>input").value
  if (val.length>=8 && val.length<=512 && /[\s~`!@#$%\^&*+=\-\[\]\\';,/{}|\\":<>\?()\._]/g.test(val) && /[0-9]/g.test(val)) {
    document.querySelector("#password>div.buttons>a.button").removeAttribute("disabled")
  } else {
    document.querySelector("#password>div.buttons>a.button").setAttribute("disabled", "")
  }
}

let PasswordLogin = () => {
  let LoginAttempt = new XMLHttpRequest()
  LoginAttempt.open("POST", "/api/user/login", true)
  LoginAttempt.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
  LoginAttempt.send("email="+Email+"&password="+document.querySelector("#password>.input>input").value+"&browser="+browser+"&os="+os)
  LoginAttempt.onreadystatechange = () => {
    if (LoginAttempt.readyState == 4) {
      console.log(LoginAttempt.responseText)
      let Response = JSON.parse(LoginAttempt.responseText)
      if (Response.PhoneNumbers!=undefined&&Response.PhoneNumbers.length>0) {
        for (let i=0;i<Response.PhoneNumbers.length;i++) {
          let Selection = document.createElement("div")
          Selection.classList.add("select")
          Selection.innerHTML = '<svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="1"><path stroke-linecap="round" stroke-linejoin="round" d="M12 18h.01M8 21h8a2 2 0 002-2V5a2 2 0 00-2-2H8a2 2 0 00-2 2v14a2 2 0 002 2z" /></svg><span>'+Response.PhoneNumbers[i][0]+'</span>'
          Selection.onclick = () => {
            CodeFAType = "phone"
            CodeFAID = Response.PhoneNumbers[i][1]
            document.getElementById("code-fa-select").classList.toggle("hidden")
            document.getElementById("code-fa").classList.toggle("hidden")
            console.log(CodeFAID)
          }
          document.querySelector("#code-fa-select").appendChild(Selection)
        }
        let Back = document.createElement("a")
        Back.classList.add("button-secondary")
        Back.innerText = "Back"
        Back.onclick = () => {
          document.getElementById("code-fa-select").classList.toggle("hidden")
          document.getElementById("password").classList.toggle("hidden")
          let Selections = document.querySelectorAll("#code-fa-select>.select")
          for (let i=0;i<Selections.length;i++) Selections[i].remove()
          Back.remove()
        }
        document.querySelector("#code-fa-select").appendChild(Back)
        document.getElementById("password").classList.toggle("hidden")
        document.getElementById("code-fa-select").classList.toggle("hidden")
      }
    }
  }
}

document.querySelector("#password>div.buttons>.button").onclick = () => PasswordLogin()
document.querySelector("#password>.input>input").onkeydown = (e) => { if (e.keyCode == 13) PasswordLogin() }

// New login.js - 2FA
document.querySelector("#code-fa>div.buttons>a.button-secondary").onclick = () => {
  document.getElementById("code-fa").classList.toggle("hidden")
  document.getElementById("code-fa-select").classList.toggle("hidden")
}

let CodeFAWrapper = document.getElementById("code-fa-wrapper")
let CodeFA1 = document.getElementById("code-fa-1")
let CodeFA2 = document.getElementById("code-fa-2")
let CodeFA3 = document.getElementById("code-fa-3")
let CodeFA4 = document.getElementById("code-fa-4")

let CodeFAError = () => {
  for (let i=0;i<CodeFAWrapper.childElementCount;i++) CodeFAWrapper.children[i].style.border = "1px solid red"
  CodeFA4.focus()
}

let CodeFAChange = (now, next) => {
  if (now.value.length > 1) now.value = now.value[0]
  if (CodeFA1.value.length == 1 && CodeFA2.value.length == 1 && CodeFA3.value.length == 1 && CodeFA4.value.length == 1) document.getElementById("next").removeAttribute("disabled")
  else document.getElementById("next").setAttribute("disabled", "")
  next.focus()
}

let CodeFABackspace = (now, prev, e) => {
  if (e.keyCode == 8 && now.value.length == 0) {
    prev.focus()
    prev.value = ""
    e.preventDefault()
  } else if (e.keyCode == 8) {
    now.value = ""
    now.focus()
  }
  if (CodeFA1.value.length == 0 || CodeFA2.value.length == 0 || CodeFA3.value.length == 0 || CodeFA4.value.length == 0) document.getElementById("next").setAttribute("disabled", "")
}

let CodeFALogin = () => {
  let LoginAttempt = new XMLHttpRequest()
  LoginAttempt.open("POST", "/api/user/login", true)
  LoginAttempt.setRequestHeader("Content-Type", "application/x-www-form-urlencoded")
  LoginAttempt.send("email="+Email+"&password=" + document.querySelector("#password>.input>input").value + "&id=" + CodeFAID + "&code="+CodeFA1.value+CodeFA2.value+CodeFA3.value+CodeFA4.value)
  console.log("email="+Email+"&password=" + document.querySelector("#password>.input>input").value + "&id=" + CodeFAID + "&code="+CodeFA1.value+CodeFA2.value+CodeFA3.value+CodeFA4.value)
  LoginAttempt.onreadystatechange = () => {
    if (LoginAttempt.readyState == 4) {
      console.log(LoginAttempt.responseText)
      let Response = JSON.parse(LoginAttempt.responseText)
      if (Response.error!=undefined) {
        SentCodeMessage.innerHTML = "We've sent you a new code."
        document.getElementById("code-fa-error").innerText = "Invalid code."
        CodeFAError()
      } else location.href = "/"
    }
  }
}

CodeFA1.oninput = () => CodeFAChange(CodeFA1, CodeFA2)
CodeFA2.oninput = () => CodeFAChange(CodeFA2, CodeFA3)
CodeFA3.oninput = () => CodeFAChange(CodeFA3, CodeFA4)
CodeFA4.oninput = () => CodeFAChange(CodeFA4, CodeFA4)
CodeFA2.onkeydown = (e) => CodeFABackspace(CodeFA2, CodeFA1, e)
CodeFA3.onkeydown = (e) => CodeFABackspace(CodeFA3, CodeFA2, e)
CodeFA4.onkeydown = (e) => {
  CodeFABackspace(CodeFA4, CodeFA3, e)
  if (e.keyCode == 13) CodeFALogin()
}

document.querySelector("#code-fa>div.buttons>.button").onclick = () => CodeFALogin()

let CheckCodeMessage = document.getElementById("check-code")
let ResendCodeMessage = document.getElementById("resend-code")
let SentCodeMessage = document.getElementById("sent-code")

CheckCodeMessage.innerHTML = CheckCodeMessage.innerHTML.replace("{PhoneOrEmail}", CodeFAType == "email"? "email": "phone")
ResendCodeMessage.innerHTML = ResendCodeMessage.innerHTML.replace("{PreCodeType}", CodeFAType == "email"? "an": "a")
ResendCodeMessage.innerHTML = ResendCodeMessage.innerHTML.replace("{CodeType}", CodeFAType == "email"? "email": "text")
SentCodeMessage.innerHTML = SentCodeMessage.innerHTML.replace("{new}", CodeFARetry? "new": "")