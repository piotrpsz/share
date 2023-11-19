# share
A library of useful general-purpose functions for use in any program.

## dependencies:
### <i>fmt library</i>
<ol>
    <li>from https://github.com/fmtlib/fmt</li>
    <li>install on macOS: brew install fmt</li>
</ol>

### <i>range-v3 (Eric Niebler) library</i>
<ol>
    <li>from https://github.com/ericniebler/range-v3</li>
    <li>install on macOS: brew install range-v3</li>
</ol>

### <i>date (Howard Hinnant) library</i>
<ol>
    <li>from https://github.com/HowardHinnant/date</li>
    <li>install on macOS: brew install howard-hinnant-date</li>
</ol>

## How to add to your own project (using CMake):<br>
<ol>
    <li>Go to the root directory of your project (I assume you have git initialized).</li>
    <li>Enter the command: <b><i>git submodule add https://github.com/piotrpsz/share.git</i></b></li>
    <li>In your project's <b>CMakeLists.txt</b> file, add the following:</li>
    <ul>
        <li>add_subdirectory(share)</li>
        <li>target_link_libraries(your_project_name PUBLIC share)</li>
    </ul>
</ol>
