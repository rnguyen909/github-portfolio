

// Display a welcome message when the website loads
window.addEventListener("load", function() {
    console.log("Hello Friend, This Is My Porfoio!");
});

// Display a message when a visitor clicks a project
function projectMessage() {
    alert("Here is some of my projects");
}

// Automatically update the copyright year
const footer = document.querySelector("footer");

if (footer) {
    footer.innerHTML += "<p>Copyright &copy; " +
        new Date().getFullYear() +
        " Raymond's Portfolio. All rights reserved.</p>";
}
