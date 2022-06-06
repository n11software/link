for (var i=0;i<document.getElementsByTagName("img").length;i++) {
  if (document.getElementsByTagName("img")[i].getAttribute("nolazy")==null) {
    document.getElementsByTagName("img")[i].classList.add("loading")
    document.getElementsByTagName("img")[i].onload = (e) => {
      if (e.target.naturalHeight > 1 && e.target.complete) {
        e.target.classList.remove("loading")
      }
    }
    document.getElementsByTagName("img")[i].onerror = (e) => {
      e.target.src = 'data:image/gif;base64,R0lGODlhAQABAIAAAAAAAP\/\/\/yH5BAEAAAAALAAAAAABAAEAAAIBRAA7'
    }
  }
}