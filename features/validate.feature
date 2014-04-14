Feature: Validate a blueprint

  Scenario: Validate a valid blueprint file

    When I run `snowcrash --validate blueprint.apib`
    Then the output should contain:
    """
    OK
    """

  Scenario: Validate a valid blueprint input

    When I run `snowcrash --validate` interactively
    When I pipe in the file "blueprint.apib"
    Then the output should contain:
    """
    OK
    """

  Scenario: Validate an invalid blueprint file

    When I run `snowcrash --validate invalid_blueprint.apib`
    Then it should fail with:
    """
    error
    """

  Scenario: Validate an invalid blueprint input

    When I run `snowcrash --validate` interactively
    When I pipe in the file "invalid_blueprint.apib"
    Then it should fail with:
    """
    error
    """
