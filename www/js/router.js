let Route = (url) => {
  let HTTP = new XMLHttpRequest()
  HTTP.open('GET', url)
  HTTP.send()
  HTTP.onreadystatechange = error => {
    window.history.pushState(document.querySelector('html').innerHTML, '', url);
    document.querySelector('html').innerHTML=(HTTP.responseText)
  }
}