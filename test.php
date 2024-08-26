<?php

$before = function (... $args) {
    echo "before function called\n";
    print_r($args);
};

$after = function (... $args) {
    echo "after function called\n";
    print_r($args);
};
//
hooks_set_hook('DateTime', 'format', $before, $after);
hooks_set_hook('DateTime', 'add', $before, $after);
hooks_set_hook('sleep', $before, $after);

$date = new DateTime();

echo $date->format("l")."\n";

try{
    $date->add(DateInterval::createFromDateString('1 day'));
} catch (Exception $e) {
}

echo $date->format("l")."\n";


// sleep(1);

// sleep(1);
// sleep(1);