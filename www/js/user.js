let CurrentToken = localStorage.getItem("Token-"+localStorage.getItem("CurrentToken"))
let LoggedIn = false
let Email, UUID
let Callbacks = []
if (CurrentToken != null) {
  let Request = new XMLHttpRequest()
  Request.open("POST", "/api/user", true)
  Request.setRequestHeader("Authorization", "Bearer "+CurrentToken)
  Request.send()
  Request.onreadystatechange = () => {
    if (Request.readyState == 4) {
      LoggedIn = true
      let Response = JSON.parse(Request.responseText)
      if (Response.error == undefined) {
        Email = Response.email
        UUID = Response.uuid
        for (Callback of Callbacks) Callback()
      } else {
        window.location.href = "/login"
      }
    }
  }
}

let Register = callback => Callbacks.push(callback)
let GetEmail = () => Email
let GetUUID = () => UUID

let GetData = (token, callback) => {
  let Request = new XMLHttpRequest()
  Request.open("POST", "/api/user", true)
  Request.setRequestHeader("Authorization", "Bearer "+token)
  Request.send()
  Request.onreadystatechange = () => {
    if (Request.readyState == 4) {
      let Response = JSON.parse(Request.responseText)
      if (Response.error == undefined) {
        callback(Response)
      }
    }
  }
}

let Emails = [], UUIDs = []

let RegisterAll = callback => {
  Emails = [], UUIDs = []
  for (let i=1;i<=localStorage.getItem("Tokens");i++) {
    GetData(localStorage.getItem("Token-"+i), Response => {
      if (Response.email != undefined) {
        UUIDs.push(Response.uuid)
        Emails.push(Response.email)
        if (i == localStorage.getItem("Tokens")) callback()
      }
    })
  }
}