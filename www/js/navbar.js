let isProductsOpen = false, isEmployeesOpen = false

let setIsProductsOpen = value => {
  let navbar = document.getElementById("navbar")
  isProductsOpen = false
  isProductsOpen = value
  if (isProductsOpen) {
    document.querySelector('#products-button').classList.add("active")
    navbar.after(products)
    navbar.classList.add("normal")
    navbar.classList.remove("floating")
  } else {
    document.querySelector('#products-button').classList.remove("active")
    products.remove()
  }
}

let setIsEmployeesOpen = value => {
  isProductsOpen = false
  isEmployeesOpen = value
  if (isEmployeesOpen) document.querySelector('#employees-button').classList.add("active")
  else document.querySelector('#employees-button').classList.remove("active")
}

let setIsOpen = (value, button) => {
  if (button == 0) {
    setIsEmployeesOpen(false)
    setIsProductsOpen(value)
  } else {
    setIsProductsOpen(false)
    setIsEmployeesOpen(value)
  }
}

document.onscroll = () => {
  let navbar = document.getElementById("navbar")
  if (window.scrollY > 16) {
    navbar.classList.add("floating")
    navbar.classList.remove("normal")
  } else {
    navbar.classList.add("normal")
    navbar.classList.remove("floating")
  }
  setIsOpen(false, 0)
}